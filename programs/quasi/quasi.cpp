
#include "math/real.hpp"
#include "math/vec2.hpp"

#include "glt/ShaderProgram.hpp"
#include "glt/Mesh.hpp"
#include "glt/CubeMesh.hpp"

#include "ge/Engine.hpp"

using namespace math;

struct Vertex {
    vec2_t position;
};

DEFINE_VERTEX_DESC(Vertex,
                   VERTEX_ATTR(Vertex, position));

struct Anim {
    glt::CubeMesh<Vertex> quadBatch;
    
    void init(const ge::Event<ge::InitEvent>&);
    void link(ge::Engine& e);
    void animate(const ge::Event<ge::AnimationEvent>&);
    void renderScene(const ge::Event<ge::RenderEvent>&);
};

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    link(ev.info.engine);
    
    {
        Vertex v;
        v.position = vec2(-1.f, -1.f); quadBatch.add(v);
        v.position = vec2( 1.f, -1.f); quadBatch.add(v);
        v.position = vec2( 1.f,  1.f); quadBatch.add(v);
        v.position = vec2(-1.f,  1.f); quadBatch.add(v);

        quadBatch.send();
    }

    ev.info.success = true;
}

void Anim::link(ge::Engine& e) {
    e.events().animate.reg(ge::makeEventHandler(this, &Anim::animate));
    e.events().render.reg(ge::makeEventHandler(this, &Anim::renderScene));
}

void Anim::animate(const ge::Event<ge::AnimationEvent>&) {
    // empty
}

void Anim::renderScene(const ge::Event<ge::RenderEvent>& ev) {
    ge::Engine& e = ev.info.engine;
    real time = e.gameLoop().gameTime() + ev.info.interpolation * e.gameLoop().frameDuration();

    Ref<glt::ShaderProgram> renderShader = e.shaderManager().program("render");
    if (!renderShader) {
        ASSERT_FAIL();
    }

    renderShader->use();
    glt::Uniforms(*renderShader)
        .optional("gammaCorrection", real(2.2))
        .optional("time", time)
        .optional("transform", mat3());

    quadBatch.draw();
}


int main(int argc, char *argv[]) {
    ge::Engine engine;
    ge::EngineOptions opts;
    Anim anim;
    
    opts.parse(&argc, &argv);
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));
    return engine.run(opts);
}
