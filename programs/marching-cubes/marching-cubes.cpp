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
#include "glt/Transformations.hpp"

#include "sys/measure.hpp"

#include "math/vec4.hpp"
#include "math/ivec3.hpp"
#include "math/io.hpp"

#include "marching-cubes/tables.hpp"
#include "marching-cubes/tables2.hpp"

using namespace math;
using namespace defs;

static const vec3_t WORLD_BLOCK_SCALE = vec3(32);

static const ivec3_t SAMPLER_SIZE = ivec3(8);

static const size BLOCK_DATA_SIZE = (SAMPLER_SIZE[0] * SAMPLER_SIZE[1] * SAMPLER_SIZE[2]) * 2; // may not fit in worst case

static const ivec3_t BLOCKS = ivec3(10, 3, 10);
static const ivec3_t ORIGIN = ivec3(0, 0, 0);

typedef GLuint GLTransformFeedback;
typedef GLuint GLArrayBuffer;
typedef GLuint GLVertexArray;

struct WorldVertex {
    vec3_t position;
};

struct MCVertex {
    vec3_t position;
};

struct MCFeedbackVertex {
    vec3_t position;
    vec3_t normal;
};

DEFINE_VERTEX_DESC(WorldVertex, VERTEX_ATTR(WorldVertex, position));

DEFINE_VERTEX_DESC(MCVertex, VERTEX_ATTR(MCVertex, position));

DEFINE_VERTEX_DESC(MCFeedbackVertex,
                   VERTEX_ATTR(MCFeedbackVertex, position),
                   VERTEX_ATTR(MCFeedbackVertex, normal));

struct Block {
    vec3_t aabb_min;
    vec3_t aabb_max;
    GLTransformFeedback stream;
    GLArrayBuffer data;
    GLVertexArray array;    
};

struct Anim {
    ge::Engine *engine;
    glt::Mesh<WorldVertex> unitRect; // a slice in the world volume
    Ref<glt::TextureRenderTarget3D> worldVolume;
    glt::Mesh<MCVertex> volumeCube;

    glt::TextureHandle caseToNumPolysData;
    glt::TextureHandle triangleTableData;
    
    Ref<glt::ShaderProgram> worldProgram;
    Ref<glt::ShaderProgram> marchingCubesProgram;
    Ref<glt::ShaderProgram> renderPolygonProgram;
    
    ge::Camera camera;
    Ref<ge::Timer> fpsTimer;

    std::vector<Block> blocks;

    Anim() :
        engine(0) {}
    
    void link(ge::Engine&);
    void init(const ge::Event<ge::InitEvent>&);
    void animate(const ge::Event<ge::AnimationEvent>&);
    void render(const ge::Event<ge::RenderEvent>&);

    void initBlock(Block *block);
    void destroyBlock(Block *block);

    void makeBlock(Block& block, const vec3_t& aabb_min, const vec3_t& aabb_max);
    void makeSampleVolume();
    void makePolygon(const Block&);
    void renderPolygon(const Block&);
};

void Anim::link(ge::Engine& e) {
    engine = &e;
    e.events().animate.reg(makeEventHandler(this, &Anim::animate));
    e.events().render.reg(makeEventHandler(this, &Anim::render));
}

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    camera.registerWith(*engine);
    camera.registerCommands(engine->commandProcessor());

    engine->window().grabMouse(true);
    engine->window().showMouseCursor(false);

    {
        WorldVertex v;
        unitRect.primType(GL_TRIANGLES);
        v.position = vec3(0.f, 0.f, 0.f); unitRect.addVertex(v);
        v.position = vec3(1.f, 0.f, 0); unitRect.addVertex(v);
        v.position = vec3(0.f, 1, 0); unitRect.addVertex(v);
        v.position = vec3(0.f, 1, 0); unitRect.addVertex(v);
        v.position = vec3(1, 0.f, 0); unitRect.addVertex(v);
        v.position = vec3(1, 1, 0); unitRect.addVertex(v);
        unitRect.send();
    }

    {
        glt::TextureRenderTarget3D::Params ps;
        ps.default_filter_mode = glt::TextureHandle::FilterLinear;
        ps.color_format = GL_R32F;
        worldVolume = new glt::TextureRenderTarget3D(SAMPLER_SIZE + ivec3(1), ps);
    }

    caseToNumPolysData.type(glt::Texture1D);
    caseToNumPolysData.bind();
    
    GL_CHECK(glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, sizeof edgeTable, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, edgeTable));
    caseToNumPolysData.filterMode(glt::TextureHandle::FilterNearest);

    triangleTableData.type(glt::Texture1D);
    triangleTableData.bind();
    GL_CHECK(glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, sizeof triTable, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, triTable));
    triangleTableData.filterMode(glt::TextureHandle::FilterNearest);
    GL_CHECK(glBindTexture(GL_TEXTURE_1D, 0));

    volumeCube.primType(GL_POINTS);

    for (int i = 0; i < SAMPLER_SIZE[0]; ++i) {
        for (int j = 0; j < SAMPLER_SIZE[1]; ++j) {
            for (int k = 0; k < SAMPLER_SIZE[2]; ++k) {
                MCVertex v;
                v.position = vec3(ivec3(i, j, k)) / vec3(SAMPLER_SIZE);
                volumeCube.addVertex(v);
            }   
        }
    }

    volumeCube.send();

    worldProgram = engine->shaderManager().program("world");
    if (!worldProgram) return;

    marchingCubesProgram = new glt::ShaderProgram(engine->shaderManager());
    marchingCubesProgram->addShaderFile("marching-cubes.vert");
    marchingCubesProgram->addShaderFile("marching-cubes.geom");
    marchingCubesProgram->bindAttributes<MCVertex>();
    std::string vars[] = { "gPosition", "gNormal" };
    marchingCubesProgram->bindStreamOutVaryings(Array<std::string>(vars, ARRAY_LENGTH(vars)));
    if (!marchingCubesProgram->tryLink())
        return;

    renderPolygonProgram = engine->shaderManager().program("render-polygon");
    if (!renderPolygonProgram)
        return;

    size num_blocks = BLOCKS[0] * BLOCKS[1] * BLOCKS[2];
    blocks.resize(num_blocks);

    int id = 0;
    for (int i = 0; i < BLOCKS[0]; ++i) {
        for (int j = 0; j < BLOCKS[1]; ++j) {
            for (int k = 0; k < BLOCKS[2]; ++k, ++id) {
                ivec3_t idx = ivec3(i, j, k) + ORIGIN;
                Block *block = &blocks[id];
                initBlock(block);
                makeBlock(*block, vec3(idx), vec3(idx + ivec3(1)));
            }
        }
    }
    
    fpsTimer = new ge::Timer(*engine);
    fpsTimer->start(1.f, true);

    GL_CHECK(glFinish());

    ev.info.success = true;
}

void Anim::initBlock(Block *block) {
    ASSERT(block);
    
    GL_CHECK(glGenBuffers(1, &block->data));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, block->data));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, BLOCK_DATA_SIZE * sizeof (MCFeedbackVertex), 0, GL_STREAM_DRAW));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    GL_CHECK(glGenVertexArrays(1, &block->array));
    GL_CHECK(glBindVertexArray(block->array));
    
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, block->data));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof (MCFeedbackVertex), 0));
    GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof (MCFeedbackVertex), (void *) offsetof(MCFeedbackVertex, normal)));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glBindVertexArray(0));

    GL_CHECK(glGenTransformFeedbacks(1, &block->stream));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, block->stream));
    GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, block->data));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));
}

void Anim::destroyBlock(Block *block) {
    GL_CHECK(glDeleteTransformFeedbacks(1, &block->stream)); block->stream = 0;
    GL_CHECK(glDeleteVertexArrays(1, &block->array)); block->array = 0;
    GL_CHECK(glDeleteBuffers(1, &block->data)); block->data = 0;
}

void Anim::makeBlock(Block& block, const vec3_t& aabb_min, const vec3_t& aabb_max) {
    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
    glt::SavePoint sp(gt.save());

    vec3_t fuzz = vec3(0.f);
    vec3_t max = aabb_max + fuzz;

    block.aabb_min = aabb_min;
    block.aabb_max = max;

    gt.translate(aabb_min);
    gt.scale(max - aabb_min);
    makeSampleVolume();
    makePolygon(block);
}

void Anim::makeSampleVolume() {
    glt::RenderManager& rm = engine->renderManager();

    GL_CHECK(glDisable(GL_DEPTH_TEST));
    
    worldProgram->use();

    vec3_t tex_scale = vec3(SAMPLER_SIZE - ivec3(1)) / vec3(SAMPLER_SIZE - ivec3(2));
    mat4_t scaleM = glt::scaleMatrix(tex_scale);

    float invDim = 1.f / float(worldVolume->depth() - 1);
    for (index i = 0; i < worldVolume->depth(); ++i) {
        worldVolume->targetAttachment(glt::TextureRenderTarget3D::Attachment(
                                          glt::TextureRenderTarget3D::AttachmentLayer, i));

        rm.setActiveRenderTarget(worldVolume.ptr());
        glt::Uniforms(*worldProgram)
            .mandatory("depth", float(i) * invDim)
            .mandatory("worldMatrix", rm.geometryTransform().modelMatrix() * scaleM)
            .optional("time", engine->gameLoop().gameTime());
        
        unitRect.draw();
        rm.setActiveRenderTarget(0);
    }

    rm.setDefaultRenderTarget();
    GL_CHECK(glEnable(GL_DEPTH_TEST));
}

void Anim::makePolygon(const Block& block) {
    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
    glt::SavePoint(gt.save());

    GL_CHECK(glEnable(GL_RASTERIZER_DISCARD));

    marchingCubesProgram->use();

    caseToNumPolysData.bind(0);
    triangleTableData.bind(1);
    worldVolume->textureHandle().bind(2);

    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    glt::Uniforms(*marchingCubesProgram)
        .mandatory("caseToNumPolysData", caseToNumPolysData, 0, GL_UNSIGNED_INT_SAMPLER_1D)
        .mandatory("triangleTableData", triangleTableData, 1, GL_UNSIGNED_INT_SAMPLER_1D)
        .optional("worldVolume", worldVolume->textureHandle(), 2, GL_SAMPLER_3D)
        .mandatory("texEdgeDim", vec3(1) / vec3(SAMPLER_SIZE))
        .optional("worldMatrix", gt.modelMatrix())
//        .mandatory("projectionMatrix", gt.projectionMatrix())
        ;

    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, block.stream));
    GL_CHECK(glBeginTransformFeedback(GL_TRIANGLES));
    volumeCube.draw();
    GL_CHECK(glEndTransformFeedback());

    GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));
}

void Anim::renderPolygon(const Block& block) {
    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
    glt::SavePoint sp(gt.save());

    gt.translate(block.aabb_min);
    gt.scale(block.aabb_max - block.aabb_min);

    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        
    renderPolygonProgram->use();

    const mat4_t scaleM = glt::scaleMatrix(WORLD_BLOCK_SCALE);
    
    glt::Uniforms(*renderPolygonProgram)
        .mandatory("mvMatrix", gt.viewMatrix() * (scaleM * gt.modelMatrix()))
        .mandatory("projectionMatrix", gt.projectionMatrix())
        .mandatory("normalMatrix", gt.normalMatrix());
        
    GL_CHECK(glBindVertexArray(block.array));
    GL_CHECK(glDrawTransformFeedback(GL_TRIANGLES, block.stream));
    GL_CHECK(glBindVertexArray(0));
}

void Anim::animate(const ge::Event<ge::AnimationEvent>&) {
    
}

void Anim::render(const ge::Event<ge::RenderEvent>&) {
    GL_CHECK(glClearColor(1.f, 1.f, 1.f, 1.f));
    engine->renderManager().activeRenderTarget()->clear();

//     glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
//     {
//         float scale = 1;
//         renderBlock(vec3(-1.f) * scale, vec3(1.f) * scale);
// //        renderBlock(vec3(0.5f, 0.f, 0.f) * scale, vec3(1.f, 0.5f, 0.5f) * scale);
//     }

    
    for (int i = 0; i < blocks.size(); ++i)
        renderPolygon(blocks[i]);

    if (fpsTimer->fire()) {
        glt::FrameStatistics fs = engine->renderManager().frameStatistics();
        std::cerr << "Timings (FPS/Render Avg/Render Min/Render Max): " << fs.avg_fps << "; " << fs.rt_avg << "; " << fs.rt_min << "; " << fs.rt_max << std::endl;
    }
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
