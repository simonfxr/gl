#include "math/vec3.hpp"

#include "glt/Mesh.hpp"

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

layout(triangle_strip, max_vertices = 6) out;

in vec3 vCorner0[1];
in vec3 vCorner1[1];
in vec3 vCorner2[1];
in vec3 vCorner3[1];

vec4 transform(vec3 point) {
    return vec4(point, 1);
}

void main() {
    vec4 positions[4];
    positions[0] = transform(vCorner0[0]);
    positions[1] = transform(vCorner1[0]);
    positions[2] = transform(vCorner2[0]);
    positions[3] = transform(vCorner3[0]);
    
    gl_Position = positions[0]; EmitVertex();
    gl_Position = positions[1]; EmitVertex();
    gl_Position = positions[3]; EmitVertex();
    EndPrimitive();

    gl_Position = positions[3]; EmitVertex();
    gl_Position = positions[1]; EmitVertex();
    gl_Position = positions[2]; EmitVertex();
    EndPrimitive();
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

struct Program {
    ge::Engine& engine;
    Ref<glt::ShaderProgram> quadProgram;
    glt::Mesh<QuadVertex> quadMesh;

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
    quadProgram->addShaderSrc(FRAGMENT_SHADER, glt::ShaderManager::FragmentShader);
    quadProgram->bindAttributes<QuadVertex>();
    if (quadProgram->wasError() || !quadProgram->tryLink())
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
    
    ev.info.success = true;
}

void Program::link() {
    engine.events().render.reg(ge::makeEventHandler(this, &Program::render));
}

void Program::render(const ge::Event<ge::RenderEvent>&) {
    glt::RenderManager& rm = engine.renderManager();
    rm.activeRenderTarget()->clear();

    quadProgram->use();
    quadMesh.draw();
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
