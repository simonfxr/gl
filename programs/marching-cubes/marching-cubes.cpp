#include "ge/Camera.hpp"
#include "ge/Command.hpp"
#include "ge/Engine.hpp"
#include "ge/MouseLookPlugin.hpp"
#include "ge/Timer.hpp"

#include "glt/CubeMesh.hpp"
#include "glt/Mesh.hpp"
#include "glt/TextureRenderTarget3D.hpp"
#include "glt/Transformations.hpp"
#include "glt/primitives.hpp"
#include "glt/utils.hpp"

#include "sys/measure.hpp"

#include "math/ivec3.hpp"
#include "math/vec4.hpp"

#include "./tables.hpp"
#include "./tables2.hpp"

using namespace math;

static const vec3_t WORLD_BLOCK_SCALE = vec3(32);

static const ivec3_t SAMPLER_SIZE = ivec3(8);

static const size_t BLOCK_DATA_SIZE =
  (SAMPLER_SIZE[0] * SAMPLER_SIZE[1] * SAMPLER_SIZE[2]) *
  2; // may not fit in worst case

static const ivec3_t BLOCKS = ivec3(10, 3, 10);
static const ivec3_t ORIGIN = ivec3(0, 0, 0);

typedef GLuint GLTransformFeedback;
typedef GLuint GLArrayBuffer;
typedef GLuint GLVertexArray;

DEF_GL_MAPPED_TYPE(WorldVertex, (vec3_t, position));

DEF_GL_MAPPED_TYPE(MCVertex, (vec3_t, position));

DEF_GL_MAPPED_TYPE(MCFeedbackVertex, (vec3_t, position), (vec3_t, normal))

struct Block
{
    vec3_t aabb_min;
    vec3_t aabb_max;
    GLTransformFeedback stream;
    GLArrayBuffer data;
    GLVertexArray array;
};

struct Anim
{
    ge::Engine *engine;
    glt::Mesh<WorldVertex> unitRect; // a slice in the world volume
    std::shared_ptr<glt::TextureRenderTarget3D> worldVolume;
    glt::Mesh<MCVertex> volumeCube;

    glt::TextureSampler caseToNumPolysData;
    glt::TextureSampler triangleTableData;

    std::shared_ptr<glt::ShaderProgram> worldProgram;
    std::shared_ptr<glt::ShaderProgram> marchingCubesProgram;
    std::shared_ptr<glt::ShaderProgram> renderPolygonProgram;

    ge::Camera camera;
    ge::MouseLookPlugin mouse_look;
    std::shared_ptr<ge::Timer> fpsTimer;

    std::vector<Block> blocks;

    Anim() : engine(nullptr) {}

    void link(ge::Engine &);
    void init(const ge::Event<ge::InitEvent> &);
    void render(const ge::Event<ge::RenderEvent> &);

    void initBlock(Block *block);
    static void destroyBlock(Block *block);

    void makeBlock(Block &block,
                   const vec3_t &aabb_min,
                   const vec3_t &aabb_max);
    void makeSampleVolume();
    void makePolygon(const Block &);
    void renderPolygon(const Block &);
};

void
Anim::link(ge::Engine &e)
{
    engine = &e;
    e.events().render.reg(makeEventHandler(*this, &Anim::render));
}

void
Anim::init(const ge::Event<ge::InitEvent> &ev)
{
    // camera.registerWith(*engine);
    // camera.registerCommands(engine->commandProcessor());

    ev.info.engine.enablePlugin(mouse_look);
    mouse_look.camera(&camera);
    ev.info.engine.enablePlugin(camera);
    camera.frame().origin = vec3(0.f);

    //    engine->window().grabMouse(true);
    engine->window().showMouseCursor(false);

    {
        WorldVertex v;
        unitRect.primType(GL_TRIANGLES);
        v.position = vec3(0.f, 0.f, 0.f);
        unitRect.addVertex(v);
        v.position = vec3(1.f, 0.f, 0);
        unitRect.addVertex(v);
        v.position = vec3(0.f, 1, 0);
        unitRect.addVertex(v);
        v.position = vec3(0.f, 1, 0);
        unitRect.addVertex(v);
        v.position = vec3(1, 0.f, 0);
        unitRect.addVertex(v);
        v.position = vec3(1, 1, 0);
        unitRect.addVertex(v);
        unitRect.send();
    }

    worldVolume = std::make_shared<glt::TextureRenderTarget3D>(
      SAMPLER_SIZE + ivec3(1),
      glt::TextureRenderTarget3D::Params{
        .texture = { .filter_mode = glt::TextureSampler::FilterLinear },
        .color_format = GL_R32F });

    caseToNumPolysData.data()->type(glt::Texture1D);
    caseToNumPolysData.data()->bind(0, false);

    GL_CALL(glTexImage1D,
            GL_TEXTURE_1D,
            0,
            GL_R32UI,
            sizeof edgeTable,
            0,
            GL_RED_INTEGER,
            GL_UNSIGNED_SHORT,
            edgeTable);
    caseToNumPolysData.filterMode(glt::TextureSampler::FilterNearest);

    triangleTableData.data()->type(glt::Texture1D);
    triangleTableData.bind(0, false);
    GL_CALL(glTexImage1D,
            GL_TEXTURE_1D,
            0,
            GL_R32UI,
            sizeof triTable,
            0,
            GL_RED_INTEGER,
            GL_UNSIGNED_BYTE,
            triTable);
    triangleTableData.filterMode(glt::TextureSampler::FilterNearest);
    triangleTableData.unbind(0, false);

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
    if (!worldProgram)
        return;

    marchingCubesProgram =
      std::make_shared<glt::ShaderProgram>(engine->shaderManager());
    marchingCubesProgram->addShaderFile("marching-cubes.vert");
    marchingCubesProgram->addShaderFile("marching-cubes.geom");
    marchingCubesProgram->bindAttributes<MCVertex>();
    std::string vars[] = { "gPosition", "gNormal" };
    marchingCubesProgram->bindStreamOutVaryings(
      Array<std::string>(vars, ARRAY_LENGTH(vars)));
    if (!marchingCubesProgram->tryLink())
        return;

    renderPolygonProgram = engine->shaderManager().program("render-polygon");
    if (!renderPolygonProgram)
        return;

    auto num_blocks = BLOCKS[0] * BLOCKS[1] * BLOCKS[2];
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

    fpsTimer = std::make_shared<ge::Timer>(*engine);
    fpsTimer->start(1.f, true);

    GL_CALL(glFinish);

    ev.info.success = true;
}

void
Anim::initBlock(Block *block)
{
    ASSERT(block);

    GL_CALL(glGenBuffers, 1, &block->data);
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, block->data);
    GL_CALL(glBufferData,
            GL_ARRAY_BUFFER,
            BLOCK_DATA_SIZE * sizeof(MCFeedbackVertex),
            nullptr,
            GL_STREAM_DRAW);
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

    GL_CALL(glGenVertexArrays, 1, &block->array);
    GL_CALL(glBindVertexArray, block->array);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, block->data);
    GL_CALL(glVertexAttribPointer,
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(MCFeedbackVertex),
            nullptr);
    GL_CALL(glVertexAttribPointer,
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(MCFeedbackVertex),
            reinterpret_cast<void *>(offsetof(MCFeedbackVertex, normal)));
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

    GL_CALL(glEnableVertexAttribArray, 0);
    GL_CALL(glEnableVertexAttribArray, 1);
    GL_CALL(glBindVertexArray, 0);

    GL_CALL(glGenTransformFeedbacks, 1, &block->stream);
    GL_CALL(glBindTransformFeedback, GL_TRANSFORM_FEEDBACK, block->stream);
    GL_CALL(glBindBufferBase, GL_TRANSFORM_FEEDBACK_BUFFER, 0, block->data);
    GL_CALL(glBindTransformFeedback, GL_TRANSFORM_FEEDBACK, 0);
}

void
Anim::destroyBlock(Block *block)
{
    GL_CALL(glDeleteTransformFeedbacks, 1, &block->stream);
    block->stream = 0;
    GL_CALL(glDeleteVertexArrays, 1, &block->array);
    block->array = 0;
    GL_CALL(glDeleteBuffers, 1, &block->data);
    block->data = 0;
}

void
Anim::makeBlock(Block &block, const vec3_t &aabb_min, const vec3_t &aabb_max)
{
    glt::GeometryTransform &gt = engine->renderManager().geometryTransform();
    auto sp = gt.save();

    vec3_t fuzz = vec3(0.f);
    vec3_t max = aabb_max + fuzz;

    block.aabb_min = aabb_min;
    block.aabb_max = max;

    gt.translate(aabb_min);
    gt.scale(max - aabb_min);
    makeSampleVolume();
    makePolygon(block);
}

void
Anim::makeSampleVolume()
{
    glt::RenderManager &rm = engine->renderManager();

    GL_CALL(glDisable, GL_DEPTH_TEST);

    worldProgram->use();

    vec3_t tex_scale =
      vec3(SAMPLER_SIZE - ivec3(1)) / vec3(SAMPLER_SIZE - ivec3(2));
    mat4_t scaleM = glt::scaleMatrix(tex_scale);

    float invDim = 1.f / float(worldVolume->depth() - 1);
    for (auto i = size_t{ 0 }; i < worldVolume->depth(); ++i) {
        worldVolume->targetAttachment(glt::TextureRenderTarget3D::Attachment{
          glt::TextureRenderTarget3D::AttachmentLayer, i });

        rm.setActiveRenderTarget(worldVolume);
        glt::Uniforms(*worldProgram)
          .mandatory("depth", float(i) * invDim)
          .mandatory("worldMatrix",
                     rm.geometryTransform().modelMatrix() * scaleM)
          .optional("time", engine->gameLoop().tickTime());

        unitRect.draw();
        rm.setActiveRenderTarget(nullptr);
    }

    rm.setDefaultRenderTarget();
    GL_CALL(glEnable, GL_DEPTH_TEST);
}

void
Anim::makePolygon(const Block &block)
{
    glt::GeometryTransform &gt = engine->renderManager().geometryTransform();
    auto sp = gt.save();

    GL_CALL(glEnable, GL_RASTERIZER_DISCARD);

    marchingCubesProgram->use();

    caseToNumPolysData.bind(0);
    triangleTableData.bind(1);
    worldVolume->sampler().bind(2);

    GL_CALL(
      glTexParameteri, GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL_CALL(
      glTexParameteri, GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CALL(
      glTexParameteri, GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glt::Uniforms(*marchingCubesProgram)
      .mandatory(
        "caseToNumPolysData",
        glt::Sampler(caseToNumPolysData, 0, GL_UNSIGNED_INT_SAMPLER_1D))
      .mandatory("triangleTableData",
                 glt::Sampler(triangleTableData, 1, GL_UNSIGNED_INT_SAMPLER_1D))
      .optional("worldVolume",
                glt::Sampler(worldVolume->sampler(), 2, GL_SAMPLER_3D))
      .mandatory("texEdgeDim", vec3(1) / vec3(SAMPLER_SIZE))
      .optional("worldMatrix", gt.modelMatrix())
      //        .mandatory("projectionMatrix", gt.projectionMatrix())
      ;

    GL_CALL(glBindTransformFeedback, GL_TRANSFORM_FEEDBACK, block.stream);
    GL_CALL(glBeginTransformFeedback, GL_TRIANGLES);
    volumeCube.draw();
    GL_CALL(glEndTransformFeedback, );

    GL_CALL(glDisable, GL_RASTERIZER_DISCARD);
}

void
Anim::renderPolygon(const Block &block)
{
    glt::GeometryTransform &gt = engine->renderManager().geometryTransform();
    auto sp = gt.save();

    const mat4_t model = gt.modelMatrix();
    const mat4_t scaleM = glt::scaleMatrix(WORLD_BLOCK_SCALE);
    gt.loadModelMatrix(scaleM * model);
    gt.translate(block.aabb_min);
    gt.scale(block.aabb_max - block.aabb_min);

    //    GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, GL_LINE);

    renderPolygonProgram->use();

    glt::Uniforms(*renderPolygonProgram)
      .mandatory("mvMatrix", gt.mvMatrix())
      .mandatory("projectionMatrix", gt.projectionMatrix())
      .mandatory("normalMatrix", gt.normalMatrix());

    GL_CALL(glBindVertexArray, block.array);
    GL_CALL(glDrawTransformFeedback, GL_TRIANGLES, block.stream);
    GL_CALL(glBindVertexArray, 0);
}

void
Anim::render(const ge::Event<ge::RenderEvent> &)
{
    GL_CALL(glClearColor, 1.f, 1.f, 1.f, 1.f);
    engine->renderManager().activeRenderTarget()->clear();

    //     glt::GeometryTransform& gt =
    //     engine->renderManager().geometryTransform();
    //     {
    //         float scale = 1;
    //         renderBlock(vec3(-1.f) * scale, vec3(1.f) * scale);
    // //        renderBlock(vec3(0.5f, 0.f, 0.f) * scale, vec3(1.f, 0.5f, 0.5f)
    // * scale);
    //     }

    for (auto i = size_t{ 0 }; i < blocks.size(); ++i)
        renderPolygon(blocks[i]);

    if (fpsTimer->fire()) {
        glt::FrameStatistics fs = engine->renderManager().frameStatistics();
        sys::io::stdout() << "Timings (FPS/Render Avg/Render Min/Render Max): "
                          << fs.avg << "; " << fs.avg << "; " << fs.min << "; "
                          << fs.max << "\n";
    }
}

int
main(int argc, char *argv[])
{
    ge::Engine engine;
    Anim anim;

    engine.setDevelDataDir(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR));
    anim.link(engine);
    ge::EngineOptions opts;

    // opts.window.settings.MajorVersion = 4;
    // opts.window.settings.MinorVersion = 1;

    opts.parse(&argc, &argv);
    opts.inits.reg(ge::Init, ge::makeEventHandler(anim, &Anim::init));

    return engine.run(opts);
}
