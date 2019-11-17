#include "math/vec3.hpp"

#include "glt/Mesh.hpp"
#include "glt/utils.hpp"

#include "ge/Engine.hpp"

using namespace math;

DEF_GL_MAPPED_TYPE(QuadVertex,
                   (vec3_t, corner0),
                   (vec3_t, corner1),
                   (vec3_t, corner2),
                   (vec3_t, corner3))

DEF_GL_MAPPED_TYPE(QuadStreamVertex, (vec3_t, position), (vec3_t, normal))

#define DEF_SHADER(name, ...) const std::string name = AS_STRING(__VA_ARGS__);

DEF_SHADER(
  QUAD_VERTEX_SHADER,

  in vec3 corner0;
  in vec3 corner1;
  in vec3 corner2;
  in vec3 corner3;

  out vec3 vCorner0;
  out vec3 vCorner1;
  out vec3 vCorner2;
  out vec3 vCorner3;

  void main() {
      vCorner0 = corner0;
      vCorner1 = corner1;
      vCorner2 = corner2;
      vCorner3 = corner3;
  }

);

DEF_SHADER(
  QUAD_GEOMETRY_SHADER,

  layout(points) in;

  in vec3 vCorner0[1];
  in vec3 vCorner1[1];
  in vec3 vCorner2[1];
  in vec3 vCorner3[1];

  layout(triangle_strip, max_vertices = 6) out;

  out vec3 position;
  out vec3 normal;

  vec3 transform(vec3 point) { return point; }

  void main() {
      vec3 positions[4];
      normal = vec3(0, 0, 1);
      positions[0] = transform(vCorner0[0]);
      positions[1] = transform(vCorner1[0]);
      positions[2] = transform(vCorner2[0]);
      positions[3] = transform(vCorner3[0]);

      position = positions[0];
      EmitVertex();
      position = positions[1];
      EmitVertex();
      position = positions[3];
      EmitVertex();
      EndPrimitive();

      position = positions[3];
      EmitVertex();
      position = positions[1];
      EmitVertex();
      position = positions[2];
      EmitVertex();
      EndPrimitive();
  }

);

DEF_SHADER(
  RENDER_VERTEX_SHADER,

  in vec3 position;
  in vec3 normal;

  void main() { gl_Position = vec4(position, 1); }

);

DEF_SHADER(
  FRAGMENT_SHADER,

  out vec4 color;

  void main() { color = vec4(1, 0, 0, 1); }

);

namespace {

struct Program
{
    ge::Engine &engine;
    Ref<glt::ShaderProgram> quadProgram;
    Ref<glt::ShaderProgram> renderProgram;
    glt::Mesh<QuadVertex> quadMesh;

    glt::GLTransformFeedbackObject quadStream;
    glt::GLVertexArrayObject quadStreamArray;
    glt::GLBufferObject quadStreamData;
    GLuint primitives_written;

    Program(ge::Engine &);
    void init(const ge::Event<ge::InitEvent> &);
    void link();

    void render(const ge::Event<ge::RenderEvent> &);
};

Program::Program(ge::Engine &e) : engine(e) {}

void
Program::init(const ge::Event<ge::InitEvent> &ev)
{

    quadProgram = new glt::ShaderProgram(engine.shaderManager());
    quadProgram->addShaderSrc(QUAD_VERTEX_SHADER,
                              glt::ShaderManager::VertexShader);
    quadProgram->addShaderSrc(QUAD_GEOMETRY_SHADER,
                              glt::ShaderManager::GeometryShader);
    std::string vars[] = { "position", "normal" };
    quadProgram->bindStreamOutVaryings(
      Array<std::string>(vars, ARRAY_LENGTH(vars)));
    quadProgram->bindAttributes<QuadVertex>();

    if (!quadProgram->tryLink())
        return;

    renderProgram = new glt::ShaderProgram(engine.shaderManager());
    renderProgram->addShaderSrc(RENDER_VERTEX_SHADER,
                                glt::ShaderManager::VertexShader);
    renderProgram->addShaderSrc(FRAGMENT_SHADER,
                                glt::ShaderManager::FragmentShader);
    renderProgram->bindAttributes<QuadStreamVertex>();
    if (!renderProgram->tryLink())
        return;

    {
        quadMesh.primType(GL_POINTS);
        QuadVertex v;
        v.corner0 = vec3(-0.5f, -0.5f, 0.f);
        v.corner1 = vec3(0.5f, -0.5f, 0.f);
        v.corner2 = vec3(0.5f, 0.5f, 0.f);
        v.corner3 = vec3(-0.5f, 0.5f, 0.f);

        quadMesh.addVertex(v);
        quadMesh.send();
    }

    quadStreamData.generate();
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, *quadStreamData);
    GL_CALL(glBufferData,
            GL_ARRAY_BUFFER,
            6 * sizeof(QuadStreamVertex),
            0,
            GL_STREAM_DRAW);
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

    quadStreamArray.generate();
    GL_CALL(glBindVertexArray, *quadStreamArray);
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, *quadStreamData);
    GL_CALL(glVertexAttribPointer,
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(QuadStreamVertex),
            0);
    GL_CALL(glVertexAttribPointer,
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(QuadStreamVertex),
            (void *) offsetof(QuadStreamVertex, normal));
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

    GL_CALL(glEnableVertexAttribArray, 0);
    GL_CALL(glEnableVertexAttribArray, 1);
    GL_CALL(glBindVertexArray, 0);

    quadStream.generate();
    GL_CALL(glBindTransformFeedback, GL_TRANSFORM_FEEDBACK, *quadStream);
    GL_CALL(glBindBufferBase, GL_TRANSFORM_FEEDBACK_BUFFER, 0, *quadStreamData);
    GL_CALL(glBindTransformFeedback, GL_TRANSFORM_FEEDBACK, 0);

    glt::GLQueryObject num_written_query;
    num_written_query.generate();
    GL_CALL(glEnable, GL_RASTERIZER_DISCARD);
    quadProgram->use();
    GL_CALL(glBeginQuery,
            GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,
            *num_written_query);
    GL_CALL(glBindTransformFeedback, GL_TRANSFORM_FEEDBACK, *quadStream);
    GL_CALL(glBeginTransformFeedback, GL_TRIANGLES);
    quadMesh.draw();
    GL_CALL(glEndTransformFeedback);
    GL_CALL(glEndQuery, GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    GL_CALL(glBindTransformFeedback, GL_TRANSFORM_FEEDBACK, 0);
    GL_CALL(glDisable, GL_RASTERIZER_DISCARD);

    GL_CALL(glGetQueryObjectuiv,
            *num_written_query,
            GL_QUERY_RESULT,
            &primitives_written);

    sys::io::stderr() << "num written: " << primitives_written << "\n";

    GL_CALL(glFinish);

    //    return;
    ev.info.success = true;
}

void
Program::link()
{
    engine.events().render.reg(ge::makeEventHandler(this, &Program::render));
}

void
Program::render(const ge::Event<ge::RenderEvent> &)
{
    glt::RenderManager &rm = engine.renderManager();
    rm.activeRenderTarget()->clearColor(glt::color(0xFF, 0xFF, 0xFF));
    rm.activeRenderTarget()->clear();
    renderProgram->use();
    GL_CALL(glBindVertexArray, *quadStreamArray);
    //    GL_CALL(glDrawTransformFeedbackInstanced, GL_TRIANGLES, *quadStream,
    //    1);
    GL_CALL(glDrawArraysInstanced, GL_TRIANGLES, 0, primitives_written * 3, 1);
    GL_CALL(glBindVertexArray, 0);
}

} // namespace

int
main(int argc, char *argv[])
{
    ge::Engine engine;
    engine.setDevelDataDir(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR));
    Program program(engine);
    program.link();
    ge::EngineOptions opts;
    opts.inits.reg(ge::Init, ge::makeEventHandler(&program, &Program::init));
    opts.parse(&argc, &argv);
    return engine.run(opts);
}
