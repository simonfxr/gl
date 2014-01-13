
#include "math/real.hpp"
#include "math/vec2.hpp"

#include "glt/ShaderProgram.hpp"
#include "glt/Mesh.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/TextureRenderTarget.hpp"
#include "glt/utils.hpp"

#include "shaders/quasi_constants.h"

#include "ge/Engine.hpp"

using namespace math;

struct Vertex {
    vec2_t position;
};

DEFINE_VERTEX_DESC(Vertex,
                   VERTEX_ATTR(Vertex, position));

struct Anim {
    ge::Engine engine;
    glt::CubeMesh<Vertex> quadBatch;

    float time_print_fps;
    
    void init(const ge::Event<ge::InitEvent>&);
    void link(ge::Engine& e);
    void animate(const ge::Event<ge::AnimationEvent>&);
    void renderScene(const ge::Event<ge::RenderEvent>&);
};

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    link(ev.info.engine);

    time_print_fps = 0;
    
    {
        Vertex v;
        v.position = vec2(-1.f, -1.f); quadBatch.add(v);
        v.position = vec2( 1.f, -1.f); quadBatch.add(v);
        v.position = vec2( 1.f,  1.f); quadBatch.add(v);
        v.position = vec2(-1.f,  1.f); quadBatch.add(v);

        quadBatch.send();
    }

    ge::GLContextInfo context_info;
    engine.window().contextInfo(context_info);
    if (context_info.antialiasingLevel > 0) {
        GL_CALL(glEnable, GL_MULTISAMPLE);
        engine.out() << "enableing multisampling" << sys::io::endl;
    }

    GL_CALL(glDisable, GL_DEPTH_TEST);

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
    real time = e.gameLoop().tickTime() + ev.info.interpolation * e.gameLoop().tickDuration();

    Ref<glt::ShaderProgram> renderShader = e.shaderManager().program("render");
    ASSERT(renderShader);

    renderShader->use();
    glt::Uniforms(*renderShader)
        .optional("gammaCorrection", real(2.2))
        .optional("time", time)
        .optional("transform", mat3());

    quadBatch.draw();

    time = e.gameLoop().realTime();
    if (time >= time_print_fps) {
        time_print_fps = time + 1.f;

        #define INV(x) (((x) * (x)) <= 0 ? -1 : 1.0 / (x))
        glt::FrameStatistics fs = engine.renderManager().frameStatistics();
        double fps = INV(fs.avg);
        double min = INV(fs.max);
        double max = INV(fs.min);
        double avg = INV(fs.avg);
        engine.out() << "Timings (FPS/Render Avg/Render Min/Render Max): " << fps << "; " << avg << "; " << min << "; " << max << sys::io::endl;
    }
}

int main(int argc, char *argv[]) {
    Anim anim;
    ge::EngineOptions opts;

    opts.parse(&argc, &argv);
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));
    return anim.engine.run(opts);
}
