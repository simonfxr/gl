#include "ge/Engine.hpp"
#include "ge/Camera.hpp"
#include "ge/Command.hpp"
#include "ge/CommandParams.hpp"
#include "ge/Timer.hpp"

#include "glt/TextureRenderTarget3D.hpp"
#include "glt/primitives.hpp"
#include "glt/Mesh.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/utils.hpp"

#include "sys/measure.hpp"

#include "math/vec4.hpp"
#include "math/ivec3.hpp"
#include "math/io.hpp"

#include "marching-cubes/tables.hpp"
#include "marching-cubes/tables2.hpp"

using namespace math;
using namespace defs;

static const ivec3_t PERLIN_NOISE_SIZE = ivec3(113);

static const ivec3_t SAMPLER_SIZE = ivec3(77);

struct SimpleVertex {
    vec3_t position;
};

DEFINE_VERTEX_DESC(SimpleVertex, VERTEX_ATTR(SimpleVertex, position));

void cpu_marching_cubes(glt::Mesh<SimpleVertex>&);

float worldVolume(const vec3_t&);

struct Anim {
    ge::Engine *engine;
    Ref<glt::TextureRenderTarget3D> perlinNoise;
    glt::Mesh<SimpleVertex> unitRect;
    glt::CubeMesh<SimpleVertex> unitCube;
    glt::Mesh<SimpleVertex> volumeCube;
    glt::Mesh<SimpleVertex> cpuMesh;
    float renderDepth;
    glt::TextureHandle caseToNumPolysData;
    glt::TextureHandle triangleTableData;

    ge::Camera camera;
    Ref<ge::Timer> fpsTimer;
    
    Anim() :
        engine(0),
        perlinNoise(),
        unitRect(),
        renderDepth(0) {}
    
    void link(ge::Engine&);
    bool initPerlinNoise();
    void init(const ge::Event<ge::InitEvent>&);
    void animate(const ge::Event<ge::AnimationEvent>&);
    void render(const ge::Event<ge::RenderEvent>&);

    void renderVolume(glt::TextureHandle& vol, float isoLvl);

    void cmdAddRenderDepth(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&);
};

void Anim::link(ge::Engine& e) {
    engine = &e;
    e.events().animate.reg(makeEventHandler(this, &Anim::animate));
    e.events().render.reg(makeEventHandler(this, &Anim::render));
}

bool Anim::initPerlinNoise() {
    
    {
        glt::TextureRenderTarget3D::Params ps;
        ps.default_filter_mode = glt::TextureHandle::FilterLinear;
        ps.color_format = GL_R32F;
        perlinNoise = new glt::TextureRenderTarget3D(PERLIN_NOISE_SIZE + ivec3(1), ps);
    }

    GL_CHECK(glDisable(GL_DEPTH_TEST));

    Ref<glt::ShaderProgram> perlinProg = engine->shaderManager().program("perlin-noise");
    if (!perlinProg)
        return false;

    perlinProg->use();

    for (index i = 0; i < perlinNoise->depth(); ++i) {
        perlinNoise->targetAttachment(glt::TextureRenderTarget3D::Attachment(
                                          glt::TextureRenderTarget3D::AttachmentLayer, i));

        engine->renderManager().beginScene();
        engine->renderManager().setActiveRenderTarget(perlinNoise.ptr());

        glt::Uniforms(*perlinProg).optional("depth", float(i) / (perlinNoise->depth() - 1));

        unitRect.draw();
        engine->renderManager().endScene();
    }

    engine->renderManager().setDefaultRenderTarget();

    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glFinish());
    
    return true;
}

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    camera.registerWith(*engine);
    camera.registerCommands(engine->commandProcessor());

    {
        SimpleVertex v;
        unitRect.primType(GL_TRIANGLES);
        v.position = vec3(-1.f, -1.f, 0.f); unitRect.addVertex(v);
        v.position = vec3(1.f, -1.f, 0); unitRect.addVertex(v);
        v.position = vec3(-1.f, 1, 0); unitRect.addVertex(v);
        v.position = vec3(-1.f, 1, 0); unitRect.addVertex(v);
        v.position = vec3(1, -1.f, 0); unitRect.addVertex(v);
        v.position = vec3(1, 1, 0); unitRect.addVertex(v);
        unitRect.send();
    }

    engine->window().grabMouse(true);
    engine->window().showMouseCursor(false);

    // glt::primitves::unitCubeWONormals(unitCube);
    // unitCube.send();

    // cpuMesh.primType(GL_TRIANGLES);
    // time(cpu_marching_cubes(cpuMesh));
    // cpuMesh.send();

    caseToNumPolysData.type(glt::Texture1D);
    caseToNumPolysData.bind();
    
    GL_CHECK(glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, sizeof edgeTable, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, edgeTable));
    caseToNumPolysData.filterMode(glt::TextureHandle::FilterNearest);

    triangleTableData.type(glt::Texture1D);
    triangleTableData.bind();
    GL_CHECK(glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, sizeof triTable, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, triTable));
    triangleTableData.filterMode(glt::TextureHandle::FilterNearest);

    volumeCube.primType(GL_POINTS);

    ivec3_t volCubeSz = SAMPLER_SIZE;
    for (int i = 0; i < volCubeSz[0]; ++i) {
        for (int j = 0; j < volCubeSz[1]; ++j) {
            for (int k = 0; k < volCubeSz[2]; ++k) {
                SimpleVertex v;
                v.position = vec3(ivec3(i, j, k)) / vec3(volCubeSz - ivec3(1));
                volumeCube.addVertex(v);
            }   
        }
    }

    volumeCube.send();

    bool ok;
    time(ok = initPerlinNoise());
    if (!ok) return;

    engine->commandProcessor().define(ge::makeCommand(this, &Anim::cmdAddRenderDepth, ge::NUM_PARAMS,
                                                      "addRenderDepth"));

    fpsTimer = new ge::Timer(*engine);
    fpsTimer->start(1.f, true);

    GL_CHECK(glFinish());
    ev.info.success = true;
}

void Anim::renderVolume(glt::TextureHandle& vol, float isoLvl) {
    Ref<glt::ShaderProgram> marchingCubesProg = engine->shaderManager().program("marching-cubes");
    ASSERT(marchingCubesProg);
    
    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();

    marchingCubesProg->use();

    caseToNumPolysData.bind(0);
    triangleTableData.bind(1);
    vol.bind(2);

    glt::Uniforms(*marchingCubesProg)
        .optional("caseToNumPolysData", caseToNumPolysData, 0, GL_UNSIGNED_INT_SAMPLER_1D)
        .optional("triangleTableData", triangleTableData, 1, GL_UNSIGNED_INT_SAMPLER_1D)
        .optional("worldVolume", vol, 2, GL_SAMPLER_3D)
        .optional("mvMatrix", gt.mvMatrix())
        .optional("normalMatrix", gt.normalMatrix())
        .optional("projectionMatrix", gt.projectionMatrix())
        .optional("texEdgeDim", vec3(1) / vec3(SAMPLER_SIZE - ivec3(1)));

    volumeCube.draw();    
}

void Anim::animate(const ge::Event<ge::AnimationEvent>&) {
    
}

void Anim::render(const ge::Event<ge::RenderEvent>&) {
    // GL_CHECK(glDisable(GL_DEPTH_TEST));
    

    // Ref<glt::ShaderProgram> noiseProg = engine->shaderManager().program("render-noise");
    // ASSERT(noiseProg);

    // noiseProg->use();
    // perlinNoise->textureHandle().bind(0);
    
    // glt::Uniforms(*noiseProg)
    //     .optional("noise", perlinNoise->textureHandle(), 0)
    //     .optional("depth", renderDepth);

    // unitRect.draw();

    GL_CHECK(glClearColor(1.f, 1.f, 1.f, 1.f));
    engine->renderManager().activeRenderTarget()->clear();

    // // GL_CHECK(glPointSize(2));

    // // Ref<glt::ShaderProgram> idProg = engine->shaderManager().program("identity");
    // // ASSERT(idProg);
    // // idProg->use();
    // // glt::Uniforms(*idProg)
    // //     .optional("mvpMatrix", engine->renderManager().geometryTransform().mvpMatrix())
    // //     .optional("color", vec4(1, 0, 0, 1));

    // // cpuMesh.draw();

    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
    {
        glt::SavePoint sp(gt.save());
        
        gt.scale(vec3(133));
        
        renderVolume(perlinNoise->textureHandle(), 0.2f);
        
        // Ref<glt::ShaderProgram> identityProg = engine->shaderManager().program("identity");
        // ASSERT(identityProg);
        // identityProg->use();
        // glt::Uniforms(*identityProg)
        //     .optional("mvpMatrix", gt.mvpMatrix())
        //     .optional("color", vec4(1, 0, 0, 1));

        // volumeCube.draw();
    }

    if (fpsTimer->fire()) {
        glt::FrameStatistics fs = engine->renderManager().frameStatistics();
        std::cerr << "Timings (FPS/Render Avg/Render Min/Render Max): " << fs.avg_fps << "; " << fs.rt_avg << "; " << fs.rt_min << "; " << fs.rt_max << std::endl;
    }
}

void Anim::cmdAddRenderDepth(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>& args) {
    renderDepth = clamp(real(renderDepth + args[0].number), 0.f, 1.f);
    std::cerr << "renderDepth: " << renderDepth << std::endl;
}

vec3_t interpolate(const vec3_t& w1, float y1, const vec3_t& w2, float y2) {
    ASSERT((y1 < 0) != (y2 < 0));
    float t = y1 / (y1 - y2);
    ASSERT(t >= 0 && t <= 1);
    return w1 + t * (w2 - w1);
}

void cpu_marching_cubes(glt::Mesh<SimpleVertex>& mesh) {
    const ivec3_t samples = ivec3(33);
    const vec3_t sample_nll = vec3(-10);
    const vec3_t sample_fur = vec3(10);

    const vec3_t step = (sample_fur - sample_nll) / vec3(samples - ivec3(1));

    for (vec3_t wc0 = sample_nll; wc0[0] < sample_fur[0]; wc0[0] += step[0]) {
        for (wc0[1] = sample_nll[1]; wc0[1] < sample_fur[1]; wc0[1] += step[1]) {
            for (wc0[2] = sample_nll[2]; wc0[2] < sample_fur[2]; wc0[2] += step[2]) {
                
                vec3_t wc = wc0;
                vec3_t wcs[8];
                float vs[8];

                wcs[0] = wc;
                wcs[1] = wc + vec3(step[0], 0, 0);
                wcs[2] = wc + vec3(step[0], 0, step[2]);
                wcs[3] = wc + vec3(0, 0, step[2]);
                wcs[4] = wc + vec3(0, step[1], 0);
                wcs[5] = wc + vec3(step[0], step[1], 0);
                wcs[6] = wc + step;
                wcs[7] = wc + vec3(0, step[1], step[2]);

                for (int i = 0; i < 8; ++i)
                    vs[i] = worldVolume(wcs[i]);

                int cubeIndex = 0;
                for (int i = 0; i < 8; ++i)
                    cubeIndex |= (vs[i] < 0) << i;
                

                //check if its completely inside or outside
                /*(step 5)*/ if(!edgeTable[cubeIndex]) continue;
                                
                //get linearly interpolated vertices on edges and save into the array
                vec3_t intVerts[12];
                /*(step 6)*/
                if(edgeTable[cubeIndex] & 1) intVerts[0] = interpolate(wcs[0], vs[0], wcs[1], vs[1]);
                if(edgeTable[cubeIndex] & 2) intVerts[1] = interpolate(wcs[1], vs[1], wcs[2], vs[2]);
                if(edgeTable[cubeIndex] & 4) intVerts[2] = interpolate(wcs[2], vs[2], wcs[3], vs[3]);
                if(edgeTable[cubeIndex] & 8) intVerts[3] = interpolate(wcs[3], vs[3], wcs[0], vs[0]);
                if(edgeTable[cubeIndex] & 16) intVerts[4] = interpolate(wcs[4], vs[4], wcs[5], vs[5]);
                if(edgeTable[cubeIndex] & 32) intVerts[5] = interpolate(wcs[5], vs[5], wcs[6], vs[6]);
                if(edgeTable[cubeIndex] & 64) intVerts[6] = interpolate(wcs[6], vs[6], wcs[7], vs[7]);
                if(edgeTable[cubeIndex] & 128) intVerts[7] = interpolate(wcs[7], vs[7], wcs[4], vs[4]);
                if(edgeTable[cubeIndex] & 256) intVerts[8] = interpolate(wcs[0], vs[0], wcs[4], vs[4]);
                if(edgeTable[cubeIndex] & 512) intVerts[9] = interpolate(wcs[1], vs[1], wcs[5], vs[5]);
                if(edgeTable[cubeIndex] & 1024) intVerts[10] = interpolate(wcs[2], vs[2], wcs[6], vs[6]);
                if(edgeTable[cubeIndex] & 2048) intVerts[11] = interpolate(wcs[3], vs[3], wcs[7], vs[7]);

                for (int i = 0; triTable[cubeIndex][i] != 255; i += 3) {
                    for (int j = 0; j < 3; ++j) {
                        SimpleVertex v;
                        v.position = intVerts[triTable[cubeIndex][i + j]];
                        mesh.addVertex(v);
                    }
                }
            }
        }
    }

    std::cerr << "mesh created with " << mesh.verticesSize() << " vertices" << std::endl;
}

// const int edge_to_verts[24] = {
//     0, 1, 1, 2,2, 3,
//     3, 0, 4, 5,5, 6,
//     6, 7, 7, 4,0, 4,
//     1, 5, 2, 6,3, 7
// };

// ivec3_t lookup_edge(int e) {
//     return ivec3(edge_to_verts[e * 2], edge_to_verts[e * 2 + 1], 0);
// }

// void cpu_marching_cubes(glt::Mesh<SimpleVertex>& mesh) {
//     const ivec3_t samples = ivec3(111);
//     const vec3_t sample_nll = vec3(-10);
//     const vec3_t sample_fur = vec3(10);

//     const vec3_t step = (sample_fur - sample_nll) / vec3(samples - ivec3(1));

//     for (vec3_t wc0 = sample_nll; wc0[0] < sample_fur[0]; wc0[0] += step[0]) {
//         for (wc0[1] = sample_nll[1]; wc0[1] < sample_fur[1]; wc0[1] += step[1]) {
//             for (wc0[2] = sample_nll[2]; wc0[2] < sample_fur[2]; wc0[2] += step[2]) {
//                 vec3_t wc = wc0;

//                 vec3_t wcs[8];
//                 float vs[8];

//                 wcs[0] = wc; wc[1] += step[1];
//                 wcs[1] = wc; wc[0] += step[0];
//                 wcs[2] = wc; wc[1] -= step[1];
//                 wcs[3] = wc; wc[0] -= step[0]; wc[2] += step[2];
//                 wcs[4] = wc; wc[1] += step[1];
//                 wcs[5] = wc; wc[0] += step[0];
//                 wcs[6] = wc; wc[1] -= step[1];
//                 wcs[7] = wc;

//                 for (int i = 0; i < 8; ++i)
//                     vs[i] = worldVolume(wcs[i]);

//                 int cas = 0;
//                 for (int i = 0; i < 8; ++i)
//                     cas |= (vs[i] < 0) << i;

//                 if (cas == 0 || cas == 255)
//                     continue;

//                 int ntri = case_to_numpolys[cas];
//                 for (int i = 0; i < ntri; ++i) {
//                     ivec3_t tri = ivec3(tri_table[cas * 15 + i],
//                                         tri_table[cas * 15 + i + 1],
//                                         tri_table[cas * 15 + i + 2]);

//                     for (int i = 0; i < 3; ++i) {
//                         ivec3_t verts = lookup_edge(tri[i]);
//                         SimpleVertex v;
//                         v.position = interpolate(wcs[verts[0]], vs[verts[0]], wcs[verts[1]], vs[verts[1]]);
//                         mesh.addVertex(v);
//                     }                                        
//                 }
//             }
//         }
//     }

//     std::cerr << "mesh created with " << mesh.verticesSize() << " vertices" << std::endl;
// }

float worldVolume(const vec3_t& coord) {
    // a sphere
    float rad = 8;
    return lengthSq(coord) - rad*rad;
}

int main(int argc, char *argv[]) {
    ge::Engine engine;
    Anim anim;
    
    anim.link(engine);
    ge::EngineOptions opts;

    // opts.window.settings.MajorVersion = 4;
    // opts.window.settings.MinorVersion = 1;
    
    opts.parse(&argc, &argv);
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));

    return engine.run(opts);
}
