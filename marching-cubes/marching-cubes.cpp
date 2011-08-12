#include "ge/Engine.hpp"
#include "ge/Camera.hpp"
#include "ge/Command.hpp"
#include "ge/CommandParams.hpp"

#include "glt/TextureRenderTarget3D.hpp"
#include "glt/primitives.hpp"
#include "glt/Mesh.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/utils.hpp"

#include "sys/measure.hpp"

#include "math/ivec3.hpp"
#include "math/io.hpp"

#include "marching-cubes/tables.hpp"

using namespace math;

static const ivec3_t PERLIN_NOISE_SIZE = ivec3(64);

struct SimpleVertex {
    vec3_t position;
};

DEFINE_VERTEX_DESC(SimpleVertex, VERTEX_ATTR(SimpleVertex, position));

struct Anim {
    ge::Engine *engine;
    Ref<glt::TextureRenderTarget3D> perlinNoise;
    glt::CubeMesh<SimpleVertex> unitRect;
    glt::CubeMesh<SimpleVertex> unitCube;
    glt::Mesh<SimpleVertex> volumeCube;
    float renderDepth;
    glt::TextureHandle caseToNumPolysData;
    glt::TextureHandle triangleTableData;

    ge::Camera camera;
    
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
        perlinNoise = new glt::TextureRenderTarget3D(PERLIN_NOISE_SIZE, ps);
    }
    
    GL_CHECK(glDisable(GL_DEPTH_TEST));

    Ref<glt::ShaderProgram> perlinProg = engine->shaderManager().program("perlin-noise");
    if (!perlinProg)
        return false;

    perlinProg->use();

    for (index_t i = 0; i < perlinNoise->depth(); ++i) {
        perlinNoise->targetDepth(i);

        engine->renderManager().beginScene();
        engine->renderManager().setActiveRenderTarget(perlinNoise.ptr());

        glt::Uniforms(*perlinProg).optional("depth", float(i) / (perlinNoise->depth() - 1));

        unitRect.draw();
        engine->renderManager().endScene();
    }

    engine->renderManager().setDefaultRenderTarget();

    GL_CHECK(glFinish());
    
    return true;
}

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    camera.registerWith(*engine);
    camera.registerCommands(engine->commandProcessor());

    glt::primitives::rectangle(unitRect, vec3(-1, -1, 0), vec2(2, 2));
    unitRect.send();

    engine->window().grabMouse(true);
    engine->window().showMouseCursor(false);

    // glt::primitves::unitCubeWONormals(unitCube);
    // unitCube.send();

    caseToNumPolysData.type(glt::Texture1D);
    caseToNumPolysData.bind();
    GL_CHECK(glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, ARRAY_LENGTH(case_to_numpolys), 0, GL_RED, GL_UNSIGNED_BYTE, case_to_numpolys));
    caseToNumPolysData.filterMode(glt::TextureHandle::FilterNearest);

    triangleTableData.type(glt::Texture1D);
    triangleTableData.bind();
    GL_CHECK(glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB8, ARRAY_LENGTH(tri_table) / 3, 0, GL_RGB, GL_BYTE, tri_table));
    triangleTableData.filterMode(glt::TextureHandle::FilterNearest);

    volumeCube.primType(GL_POINTS);

    ivec3_t volCubeSz = ivec3(32);
    for (int i = 0; i < volCubeSz[0]; ++i) {
        for (int j = 0; j < volCubeSz[1]; ++j) {
            for (int k = 0; k < volCubeSz[2]; ++k) {
                SimpleVertex v;
                v.position = vec3(i, j, k) / vec3(volCubeSz - ivec3(1));
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

    GL_CHECK(glFinish());
    ev.info.success = true;
}

void Anim::renderVolume(glt::TextureHandle& vol, float isoLvl) {
    Ref<glt::ShaderProgram> marchingCubesProg = engine->shaderManager().program("marching-cubes");
    ASSERT(marchingCubesProg);
    
    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
    const mat4_t& model = gt.modelMatrix();
    vec3_t edge_dim = (transformPoint(model, vec3(1.f)) - transformPoint(model, vec3(0.f))) / vec3(32);

    marchingCubesProg->use();

    caseToNumPolysData.bind(0);
    triangleTableData.bind(1);
    vol.bind(2);
    
    glt::Uniforms(*marchingCubesProg)
        .optional("caseToNumPolysData", caseToNumPolysData, 0, GL_INT_SAMPLER_1D)
        .optional("triangleTableData", triangleTableData, 1, GL_INT_SAMPLER_1D)
        .optional("worldVolume", vol, 2)
        .optional("modelMatrix", model)
        .optional("vpMatrix", gt.vpMatrix())
        .optional("edgeDim", edge_dim)
        .optional("texEdgeDim", vec3(1) / vec3(32))
        .optional("isoLevel", isoLvl);

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

    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
    {
        glt::SavePoint sp(gt.save());
        
        gt.scale(vec3(16));
        
        renderVolume(perlinNoise->textureHandle(), 0.2f);
        
        // Ref<glt::ShaderProgram> identityProg = engine->shaderManager().program("identity");
        // ASSERT(identityProg);
        // identityProg->use();
        // glt::Uniforms(*identityProg)
        //     .optional("mvpMatrix", gt.mvpMatrix())
        //     .optional("color", vec4(1, 0, 0, 1));

        // volumeCube.draw();
    }
}

void Anim::cmdAddRenderDepth(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>& args) {
    renderDepth = clamp(renderDepth + args[0].number, 0, 1);
    std::cerr << "renderDepth: " << renderDepth << std::endl;
}

int main(int argc, char *argv[]) {
    ge::Engine engine;
    Anim anim;
    
    anim.link(engine);
    ge::EngineOptions opts;

    opts.window.settings.MajorVersion = 4;
    opts.window.settings.MinorVersion = 1;
    
    opts.parse(&argc, &argv);
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));

    return engine.run(opts);
}
