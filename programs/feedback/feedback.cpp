#include "math/vec3.hpp"

#include "glt/Mesh.hpp"
#include "glt/utils.hpp"

#include "ge/Engine.hpp"

using namespace math;

struct QuadVertex {
    vec3_t corner0;
    vec3_t corner1;
    vec3_t corner2;
    vec3_t corner3;
};

DEFINE_VERTEX_DESC(QuadVertex,
                   VERTEX_ATTR(QuadVertex, corner0),
                   VERTEX_ATTR(QuadVertex, corner1),
                   VERTEX_ATTR(QuadVertex, corner2),
                   VERTEX_ATTR(QuadVertex, corner3));

struct QuadStreamVertex {
    vec3_t position;
    vec3_t normal;
};

DEFINE_VERTEX_DESC(QuadStreamVertex,
                   VERTEX_ATTR(QuadStreamVertex, position),
                   VERTEX_ATTR(QuadStreamVertex, normal));

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

vec3 transform(vec3 point) {
    return point;
}

void main() {
    vec3 positions[4];
    normal = vec3(0, 0, 1);
    positions[0] = transform(vCorner0[0]);
    positions[1] = transform(vCorner1[0]);
    positions[2] = transform(vCorner2[0]);
    positions[3] = transform(vCorner3[0]);
    
    position = positions[0]; EmitVertex();
    position = positions[1]; EmitVertex();
    position = positions[3]; EmitVertex();
    EndPrimitive();

    position = positions[3]; EmitVertex();
    position = positions[1]; EmitVertex();
    position = positions[2]; EmitVertex();
    EndPrimitive();
}

);


DEF_SHADER(
RENDER_VERTEX_SHADER,

in vec3 position;
in vec3 normal;

void main() {
    gl_Position = vec4(position, 1);
}

);

DEF_SHADER(
FRAGMENT_SHADER,

out vec4 color;

void main() {
    color = vec4(1, 0, 0, 1);
}
    
);

namespace {

typedef GLuint GLTransformFeedback;
typedef GLuint GLArrayBuffer;
typedef GLuint GLVertexArray;

struct Program {
    ge::Engine& engine;
    Ref<glt::ShaderProgram> quadProgram;
    Ref<glt::ShaderProgram> renderProgram;
    glt::Mesh<QuadVertex> quadMesh;

    GLTransformFeedback quadStream;
    GLVertexArray quadStreamArray;
    GLArrayBuffer quadStreamData;

    Program(ge::Engine&);
    void init(const ge::Event<ge::InitEvent>&);
    void link();

    void render(const ge::Event<ge::RenderEvent>&);
};

Program::Program(ge::Engine& e) :
    engine(e)
{}

void Program::init(const ge::Event<ge::InitEvent>& ev) {

    quadProgram = new glt::ShaderProgram(engine.shaderManager());
    quadProgram->addShaderSrc(QUAD_VERTEX_SHADER, glt::ShaderManager::VertexShader);
    quadProgram->addShaderSrc(QUAD_GEOMETRY_SHADER, glt::ShaderManager::GeometryShader);
    std::string vars[] = { "position", "normal" };
    quadProgram->bindStreamOutVaryings(Array<std::string>(vars, ARRAY_LENGTH(vars)));
    quadProgram->bindAttributes<QuadVertex>();

    if (!quadProgram->tryLink())
        return;

    renderProgram = new glt::ShaderProgram(engine.shaderManager());
    renderProgram->addShaderSrc(RENDER_VERTEX_SHADER, glt::ShaderManager::VertexShader);
    renderProgram->addShaderSrc(FRAGMENT_SHADER, glt::ShaderManager::FragmentShader);
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

    GL_CHECK(glGenBuffers(1, &quadStreamData));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, quadStreamData));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, 6 * sizeof (QuadStreamVertex), 0, GL_STREAM_DRAW));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    GL_CHECK(glGenVertexArrays(1, &quadStreamArray));
    GL_CHECK(glBindVertexArray(quadStreamArray));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, quadStreamData));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof (QuadStreamVertex), 0));
    GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof (QuadStreamVertex), (void *) offsetof(QuadStreamVertex, normal)));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glBindVertexArray(0));

    GL_CHECK(glGenTransformFeedbacks(1, &quadStream));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, quadStream));
    GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, quadStreamData));
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));

    GL_CHECK(glEnable(GL_RASTERIZER_DISCARD));
    quadProgram->use();
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, quadStream));
    GL_CHECK(glBeginTransformFeedback(GL_TRIANGLES));
    quadMesh.draw();
    GL_CHECK(glEndTransformFeedback());
    GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));
    GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));

    GL_CHECK(glFinish());

//    return;
    ev.info.success = true;
}

void Program::link() {
    engine.events().render.reg(ge::makeEventHandler(this, &Program::render));
}

void Program::render(const ge::Event<ge::RenderEvent>&) {
    glt::RenderManager& rm = engine.renderManager();
    rm.activeRenderTarget()->clear();
    renderProgram->use();
    GL_CHECK(glBindVertexArray(quadStreamArray));
    GL_CHECK(glDrawTransformFeedback(GL_TRIANGLES, quadStream));
    GL_CHECK(glBindVertexArray(0));
}

} // namespace

int main(int argc, char *argv[]) {
    ge::Engine engine;
    Program program(engine);
    program.link();
    ge::EngineOptions opts;
    opts.inits.reg(ge::Init, ge::makeEventHandler(&program, &Program::init));
    opts.parse(&argc, &argv);
    return engine.run(opts);
}
