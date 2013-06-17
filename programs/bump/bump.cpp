#include "ge/Engine.hpp"
#include "ge/Camera.hpp"

#include "glt/primitives.hpp"
#include "glt/Mesh.hpp"

using namespace defs;
using namespace math;

struct Vertex {
    vec3_t position;
    vec3_t normal;
};

DEFINE_VERTEX_DESC(Vertex,
    VERTEX_ATTR(Vertex, position),
    VERTEX_ATTR(Vertex, normal)
);

struct Anim {
    ge::Engine engine;
    glt::Mesh<Vertex> sphere_model;
    ge::Camera camera;
    vec3_t light_position;
        
    void init(const ge::Event<ge::InitEvent>&);
    void link();

    void renderScene(const ge::Event<ge::RenderEvent>&);
};

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    engine.window().showMouseCursor(false);
    engine.window().grabMouse(true);

    glt::primitives::sphere(sphere_model, 10.f, 20, 10);
    sphere_model.send();

    light_position = vec3(0.f, 0.f, -100.f);

    camera.frame.origin = vec3(0.f);
    camera.registerWith(engine);
    camera.registerCommands(engine.commandProcessor());
    
    ev.info.success = true;
}

void Anim::link() {
    engine.events().render.reg(ge::makeEventHandler(this, &Anim::renderScene));
}

void Anim::renderScene(const ge::Event<ge::RenderEvent>&) {
    glt::RenderTarget *rt = engine.renderManager().activeRenderTarget();
    rt->clearColor(glt::color(vec4(real(1))));
    rt->clear();

    Ref<glt::ShaderProgram> program = engine.shaderManager().program("sphere");
    ASSERT_MSG(program, "sphere program not found");
    glt::RenderManager& rm = engine.renderManager();

    program->use();
    glt::Uniforms(*program)
        .optional("mvpMatrix", rm.geometryTransform().mvpMatrix())
        .optional("mvMatrix", rm.geometryTransform().mvMatrix())
        .optional("normalMatrix", rm.geometryTransform().normalMatrix())
        .optional("ecLight", rm.geometryTransform().transformPoint(light_position));
    sphere_model.draw();
}

int main(int argc, char *argv[]) {
    ge::EngineOptions opts;
    Anim anim;
    anim.link();
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));
    opts.parse(&argc, &argv);
    return anim.engine.run(opts);
}
