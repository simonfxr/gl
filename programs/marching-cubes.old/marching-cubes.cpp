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

#include "err/err.hpp"
#include "sys/measure.hpp"

#include "math/vec4.hpp"
#include "math/ivec3.hpp"
#include "math/io.hpp"

#include "marching-cubes/tables.hpp"
#include "marching-cubes/tables2.hpp"

using namespace math;
using namespace defs;

static const vec3_t WORLD_BLOCK_SCALE = vec3(32);

static const ivec3_t SAMPLER_SIZE = ivec3(32);

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

struct Anim {
    ge::Engine *engine;
    glt::Mesh<WorldVertex> unitRect; // a slice in the world volume
    Ref<glt::TextureRenderTarget3D> worldVolume;
    glt::Mesh<MCVertex> volumeCube;

    glt::TextureSampler caseToNumPolysData;
    glt::TextureSampler triangleTableData;
    
    Ref<glt::ShaderProgram> worldProgram;
    Ref<glt::ShaderProgram> marchingCubesProgram;
    Ref<glt::ShaderProgram> feedbackProgram;
    
    ge::Camera camera;
    Ref<ge::Timer> fpsTimer;

    Anim() :
        engine(0) {}
    
    void link(ge::Engine&);
    void init(const ge::Event<ge::InitEvent>&);
    void animate(const ge::Event<ge::AnimationEvent>&);
    void render(const ge::Event<ge::RenderEvent>&);

    void renderBlock(const vec3_t& aabb_min, const vec3_t& aabb_max);
    void renderWorld();
    void renderVolume();
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
        ps.filter_mode = glt::TextureSampler::FilterLinear;
        ps.color_format = GL_R32F;
        worldVolume = new glt::TextureRenderTarget3D(SAMPLER_SIZE + ivec3(1), ps);
    }

    {
        caseToNumPolysData.data()->type(glt::Texture1D);
        caseToNumPolysData.bind(0);
    
        GL_CALL(glTexImage1D, GL_TEXTURE_1D, 0, GL_R32UI, sizeof edgeTable, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, edgeTable);
        caseToNumPolysData.filterMode(glt::TextureSampler::FilterNearest);
        caseToNumPolysData.unbind(0);
    }

    {
        triangleTableData.data()->type(glt::Texture1D);
        triangleTableData.bind(0);
        GL_CALL(glTexImage1D, GL_TEXTURE_1D, 0, GL_R32UI, sizeof triTable, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, triTable);
        triangleTableData.filterMode(glt::TextureSampler::FilterNearest);
        caseToNumPolysData.unbind(0);
    }

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
    marchingCubesProgram->addShaderFile("marching-cubes.frag");
    marchingCubesProgram->bindAttributes<MCVertex>();

    // GLuint prog = marchingCubesProgram->program();
    // const char *vars[] = { "gPosition", "gNormal" };
    // GL_CALL(glTransformFeedbackVaryings, prog, ARRAY_LENGTH(vars), vars, GL_INTERLEAVED_ATTRIBS);
    
    if (!marchingCubesProgram->tryLink())
        return;

    feedbackProgram = engine->shaderManager().program("feedback");
    if (!feedbackProgram)
        return;
    
    fpsTimer = new ge::Timer(*engine);
    fpsTimer->start(1.f, true);

    ev.info.success = true;
}

void Anim::renderBlock(const vec3_t& aabb_min, const vec3_t& aabb_max) {
    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
    glt::SavePoint sp(gt.save());

    // GL_CALL(glBindVertexArray, 0);

    // GLuint feedback_buf;
    // GL_CALL(glGenBuffers, 1, &feedback_buf);
    // GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, feedback_buf);
    // GLsizeiptr max_size = dot(SAMPLER_SIZE, SAMPLER_SIZE) * 15 * sizeof (MCFeedbackVertex);
    // GL_CALL(glBufferData, GL_ARRAY_BUFFER, max_size, 0, GL_STREAM_DRAW);
    // GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

    // GLuint feedback;
    // GL_CALL(glGenTransformFeedbacks, 1, &feedback);
    // GL_CALL(glBindTransformFeedback, GL_TRANSFORM_FEEDBACK, feedback);
    // GL_CALL(glEnable, GL_RASTERIZER_DISCARD);
    // GL_CALL(glBindBufferBase, GL_TRANSFORM_FEEDBACK_BUFFER, 0, feedback_buf);
    // GL_CALL(glBindTransformFeedback, GL_TRANSFORM_FEEDBACK, 0);

    vec3_t fuzz = vec3(0.01f);
    vec3_t max = aabb_max + fuzz;
        
    gt.translate(aabb_min);
    gt.scale(max - aabb_min);
    renderWorld();
    renderVolume();

    // feedbackProgram->use();
    // glt::Uniforms(*feedbackProgram)
    //     .optional("viewMatrix", gt.viewMatrix())
    //     .optional("projectionMatrix", gt.projectionMatrix())
    //     .optional("normalMatrix", gt.normalMatrix());

    // GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, feedback_buf);
    // GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, sizeof (MCFeedbackVertex), 0);
    // GL_CALL(glVertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, sizeof (MCFeedbackVertex), (void *) 12);
    // GL_CALL(glDrawTransformFeedback, GL_TRIANGLES, feedback);
}

void Anim::renderWorld() {
    GL_CALL(glDisable, GL_DEPTH_TEST);
    worldProgram->use();
    glt::RenderManager& rm = engine->renderManager();

    rm.setActiveRenderTarget(0);

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
    GL_CALL(glEnable, GL_DEPTH_TEST);
}

void Anim::renderVolume() {
    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
    glt::SavePoint sp(gt.save());

    marchingCubesProgram->use();

    caseToNumPolysData.bind(0);
    triangleTableData.bind(1);
    worldVolume->sampler().bind(2);
    worldVolume->clampMode(glt::TextureSampler::ClampToEdge);

    mat4_t scaleM = glt::scaleMatrix(WORLD_BLOCK_SCALE);
    // vec3_t inv_tex_scale = vec3(SAMPLER_SIZE - ivec3(1)) / vec3(SAMPLER_SIZE);

    glt::Uniforms(*marchingCubesProgram)
        .mandatory("caseToNumPolysData", glt::Sampler(caseToNumPolysData, 0, GL_UNSIGNED_INT_SAMPLER_1D))
        .mandatory("triangleTableData", glt::Sampler(triangleTableData, 1, GL_UNSIGNED_INT_SAMPLER_1D))
        .mandatory("worldVolume", glt::Sampler(worldVolume->sampler(), 2, GL_SAMPLER_3D))
        .mandatory("texEdgeDim", vec3(1) / vec3(SAMPLER_SIZE))
        .mandatory("mvMatrix", gt.viewMatrix() * (scaleM * gt.modelMatrix()))
        .mandatory("projectionMatrix", gt.projectionMatrix())
        .mandatory("normalMatrix", gt.normalMatrix());

    // GL_CALL(glBindTransformFeedback, GL_TRANSFORM_FEEDBACK, feedback);
    // GL_CALL(glEnable, GL_RASTERIZER_DISCARD);
    // GL_CALL(glBeginTransformFeedback, GL_TRIANGLES);
    volumeCube.draw();
    // GL_CALL(glEndTransformFeedback, );
    // GL_CALL(glDisable, GL_RASTERIZER_DISCARD);
}

void Anim::animate(const ge::Event<ge::AnimationEvent>&) {
    
}

void Anim::render(const ge::Event<ge::RenderEvent>&) {
    engine->renderManager().activeRenderTarget()->clearColor(glt::color(1.f, 1.f, 1.f));
    engine->renderManager().activeRenderTarget()->clear();

//    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
    {
        float scale = 1;
        renderBlock(vec3(-1.f) * scale, vec3(1.f) * scale);
//        renderBlock(vec3(0.5f, 0.f, 0.f) * scale, vec3(1.f, 0.5f, 0.5f) * scale);
    }

    if (fpsTimer->fire()) {
        glt::FrameStatistics fs = engine->renderManager().frameStatistics();
        sys::io::stderr() << "Timings (FPS/Render Avg/Render Min/Render Max): " << fs.avg_fps << "; " << fs.rt_avg << "; " << fs.rt_min << "; " << fs.rt_max << sys::io::endl;
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
