#include "opengl.h"

#include "glt/utils.hpp"
#include "glt/Mesh.hpp"

#include "ge/Engine.hpp"
#include "ge/Camera.hpp"

using namespace math;

namespace {

struct Vertex {
    vec3_t position;
    glt::color color;
    static const glt::VertexDesc<Vertex> ATTRIBUTES;
};

struct State {
    glt::Mesh<Vertex> triangle;
    ge::Camera camera;
    
    void init(const ge::Event<ge::InitEvent>&);
    bool initSettings(ge::Engine&);
    void render(const ge::Event<ge::RenderEvent>&);
};

} // namespace anon

int main(int argc, char *argv[]) {
    ge::EngineOptions opts;
    opts.parse(&argc, &argv);
    ge::Engine engine;
    Ref<State> state(new State);
    
    opts.inits.reg(ge::Init, ge::makeEventHandler(state.ptr(), &State::init));
    engine.events().render.reg(ge::makeEventHandler(state.ptr(), &State::render));    

    return engine.run(opts);
}

namespace {

DEFINE_VERTEX_ATTRS(Vertex::ATTRIBUTES, Vertex,
    VERTEX_ATTR(Vertex, position),
    VERTEX_ATTR(Vertex, color)
);

void State::init(const ge::Event<ge::InitEvent>& ev) {
    ge::Engine& e = ev.info.engine;
    
    if (!initSettings(e))
        return;

    camera.registerWith(e);
    camera.registerCommands(e.commandProcessor());

    Vertex v;
    v.position = vec3(0.f, 0.5f, -1.f); triangle.addVertex(v);
    v.position = vec3(-0.5f, -0.5f, -1.f); triangle.addVertex(v);
    v.position = vec3(0.5f, -0.5f, -1.f); triangle.addVertex(v);
    triangle.send();

    ev.info.success = true;
}

bool State::initSettings(ge::Engine& e) {
    e.gameLoop().ticksPerSecond(60);
    e.gameLoop().sync(true);
    return true;
}

void State::render(const ge::Event<ge::RenderEvent>& ev) {
    ge::Engine& e = ev.info.engine;
    
    GL_CHECK(glClearColor(1.f, 0.f, 0.f, 1.f));
    e.renderManager().activeRenderTarget()->clear();

    Ref<glt::ShaderProgram> triangleProg = e.shaderManager().program("triangle");
    ASSERT(triangleProg);

    triangleProg->use();
    triangle.draw();
}

} // namespace anon
