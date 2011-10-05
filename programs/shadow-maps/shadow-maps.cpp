#include <math/vec3.hpp>

#include <glt/Mesh.hpp>

#include <ge/Engine.hpp>
#include <ge/Camera.hpp>

using namespace math;

struct Vertex1 {
    vec3_t position;
};

DEFINE_VERTEX_DESC(Vertex1, VERTEX_ATTR(Vertex1, position));

struct Anim {
    ge::Engine& engine;
    ge::Camera camera;
    glt::Mesh<Vertex1> ground;

    Anim(ge::Engine& e) : engine(e) {}
    
    void link();
    void init(const ge::Event<ge::InitEvent>&);
    
    void render(const ge::Event<ge::RenderEvent>&);
};

void Anim::link() {
    engine.events().render.reg(ge::makeEventHandler(this, &Anim::render));
    camera.registerWith(engine);
    camera.registerCommands(engine.commandProcessor());

    {
        ground.primType(GL_QUADS);
        Vertex1 v;
        real scale = 10.f;
        
        v.position = vec3(-1.f, 0.f, +1.f) * scale; ground.addVertex(v);
        v.position = vec3(+1.f, 0.f, +1.f) * scale; ground.addVertex(v);        
        v.position = vec3(+1.f, 0.f, -1.f) * scale; ground.addVertex(v);
        v.position = vec3(-1.f, 0.f, -1.f) * scale; ground.addVertex(v);

        ground.send();
    }
}

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    link();

    engine.window().grabMouse(true);
    engine.window().showMouseCursor(false);

    engine.renderManager().defaultRenderTarget()->clearColor(glt::color(vec3(1.f, 1.f, 1.f)));
    
    ev.info.success = true;
}

void Anim::render(const ge::Event<ge::RenderEvent>&) {
    glt::RenderManager& rm = engine.renderManager();
    glt::GeometryTransform& gt = rm.geometryTransform();
    rm.activeRenderTarget()->clear(glt::RT_COLOR_BUFFER | glt::RT_DEPTH_BUFFER);

    Ref<glt::ShaderProgram> shader = engine.shaderManager().program("identity");
    ASSERT(shader);

    shader->use();
    glt::Uniforms(*shader)
        .optional("mvpMatrix", gt.mvpMatrix())
        .optional("color", vec4(1.f, 0.f, 0.f, 1.f));

    ground.draw();    
}

int main(int argc, char *argv[]) {
    ge::Engine engine;
    Anim anim(engine);

    ge::EngineOptions opts;
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));

    opts.parse(&argc, &argv);

    return engine.run(opts);
}
