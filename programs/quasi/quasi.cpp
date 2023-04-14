#include "ge/Anim.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/Mesh.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/TextureRenderTarget.hpp"
#include "glt/utils.hpp"
#include "math/fvec.hpp"
#include "math/real.hpp"

#include "shaders/quasi_constants.h"

using namespace math;

DEF_GL_MAPPED_TYPE(Vertex, (vec2, position))

struct Anim final : ge::Anim<Anim>
{
    glt::CubeMesh<Vertex> quadBatch;

    ge::GameLoop::time time_print_fps{};

    void init(const ge::Event<ge::InitEvent> & /*ev*/);
    void animate(const ge::Event<ge::AnimationEvent> & /*unused*/) override;
    void render(const ge::Event<ge::RenderEvent> & /*ev*/) override;
};

void
Anim::init(const ge::Event<ge::InitEvent> &ev)
{

    time_print_fps = 0;

    {
        Vertex v{};
        v.position = vec2(-1.f, -1.f);
        quadBatch.add(v);
        v.position = vec2(1.f, -1.f);
        quadBatch.add(v);
        v.position = vec2(1.f, 1.f);
        quadBatch.add(v);
        v.position = vec2(-1.f, 1.f);
        quadBatch.add(v);

        quadBatch.send();
    }

    if (engine.window().contextInfo().antialiasingLevel > 0) {
        GL_CALL(glEnable, GL_MULTISAMPLE);
        engine.out() << "enableing multisampling\n";
    }

    GL_CALL(glDisable, GL_DEPTH_TEST);

    ev.info.success = true;
}

void
Anim::animate(const ge::Event<ge::AnimationEvent> & /*unused*/)
{
    // empty
}

void
Anim::render(const ge::Event<ge::RenderEvent> &ev)
{
    ge::Engine &e = ev.info.engine;
    auto time = e.gameLoop().tickTime() +
                ev.info.interpolation * e.gameLoop().tickDuration();

    auto renderShader = e.shaderManager().program("render");
    ASSERT(renderShader);

    renderShader->use();
    glt::Uniforms(*renderShader)
      .optional("gammaCorrection", 2.2_r)
      .optional("time", time)
      .optional("transform", mat3());

    quadBatch.draw();

    time = e.gameLoop().realTime();
    if (time >= time_print_fps) {
        time_print_fps = time + 1;

#define INV(x) (((x) * (x)) <= 0 ? -1 : 1.0 / (x))
        auto fs = engine.renderManager().frameStatistics();
        auto fps = INV(fs.avg);
        auto min = INV(fs.max);
        auto max = INV(fs.min);
        auto avg = INV(fs.avg);
        engine.out() << "Timings (FPS/Render Avg/Render Min/Render Max): "
                     << fps << "; " << avg << "; " << min << "; " << max
                     << "\n";
    }
}

int
main(int argc, char *argv[])
{
    Anim anim;
    anim.engine.setDevelDataDir(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR));
    return anim.main(argc, argv);
}
