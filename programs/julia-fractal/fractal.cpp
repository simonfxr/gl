#include "ge/Anim.hpp"
#include "ge/Command.hpp"
#include "ge/Event.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/Mesh.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/TextureRenderTarget.hpp"
#include "glt/utils.hpp"
#include "math/fvec.hpp"
#include "math/real.hpp"

#include "shaders/quasi_constants.h"

static const bool MULTISAMPLING = false; // via multisample texture

using namespace math;
using namespace bl::literals;

DEF_GL_MAPPED_TYPE(Vertex, (vec2, position))

struct World
{
    real scale = 1;
    vec2 trans = vec2(0, 0);
    vec2 julia_constant = vec2(0.3_r, 0.5_r);

    void zoom(real t);
    void shift(const vec2 & /*a*/);

    void julia_constant_rotate(real phi);
    void julia_constant_scale(real t);
    void julia_constant_add(const vec2 & /*z*/);
    void julia_constant_reset();
};

struct Commands final : public ge::PluginImpl<Commands>
{
    ge::CommandPtr zoom;
    ge::CommandPtr shift_x;
    ge::CommandPtr shift_y;

    ge::CommandPtr julia_constant_rotate;
    ge::CommandPtr julia_constant_scale;
    ge::CommandPtr julia_constant_add_re;
    ge::CommandPtr julia_constant_add_im;
    ge::CommandPtr julia_constant_reset;

    explicit Commands(World &);

    void registerCommands(ge::CommandProcessor &) final;
};

struct Anim final : ge::Anim<Anim>
{
    glt::CubeMesh<Vertex> quadBatch;
    std::shared_ptr<glt::TextureRenderTarget> render_texture;

    World world;
    std::shared_ptr<Commands> commands;

    ge::GameLoop::time time_print_fps{};

    void init(const ge::Event<ge::InitEvent> & /*ev*/);
    void render(const ge::Event<ge::RenderEvent> & /*ev*/) override;

    void handleWindowResized(const ge::Event<ge::WindowResized> &ev);
};

void
Anim::init(const ge::Event<ge::InitEvent> &ev)
{
    engine->window().events().windowResized.reg(*this,
                                                &Anim::handleWindowResized);

    commands = std::make_shared<Commands>(world);
    engine->enablePlugin(*commands);

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

    if (MULTISAMPLING) {
        auto w = engine->window().windowWidth();
        auto h = engine->window().windowHeight();
        glt::TextureRenderTarget::Params ps;
        ps.samples = NUM_SAMPLES;
        ps.buffers = glt::RT_COLOR_BUFFER | glt::RT_DEPTH_BUFFER;
        render_texture = glt::TextureRenderTarget::make_shared(w, h, ps);
        engine->renderManager().setDefaultRenderTarget(render_texture);

        GL_CALL(glEnable, GL_MULTISAMPLE);
    }

    GL_CALL(glDisable, GL_DEPTH_TEST);

    ev.info.success = true;
}

void
Anim::render(const ge::Event<ge::RenderEvent> &ev)
{
    auto &e = ev.info.engine;
    auto time = e.gameLoop().tickTime() +
                ev.info.interpolation * e.gameLoop().tickDuration();

    auto &rm = engine->renderManager();

    if (MULTISAMPLING) {
        rm.setActiveRenderTarget(render_texture);
        render_texture->clear();
    }

    auto renderShader = e.shaderManager().program("render"_k);
    ASSERT(renderShader);

    renderShader->use();
    glt::Uniforms(*renderShader)
      .optional("gammaCorrection"_k, 2.2_r)
      .optional("time"_k, time)
      .optional("transform"_k, mat3())
      .optional("julia_constant"_k, world.julia_constant)
      .optional("world_shift"_k, world.trans)
      .optional("world_zoom"_k, world.scale);

    quadBatch.draw();

    if (MULTISAMPLING) {
        rm.setActiveRenderTarget(engine->window().renderTarget());
        auto postproc = e.shaderManager().program("postproc"_k);
        ASSERT(postproc);

        postproc->use();

        render_texture->sampler().bind(0);
        glt::Uniforms(*postproc).optional(
          "texture0"_k, glt::Sampler(render_texture->sampler(), 0));
        quadBatch.draw();
    }

    if (time >= time_print_fps) {
        time_print_fps = time + 1;

#define INV(x) (((x) * (x)) <= 0 ? -1. : 1.0 / (x))
        auto fs = engine->renderManager().frameStatistics();
        auto fps = INV(fs.avg);
        auto min = INV(fs.max);
        auto max = INV(fs.min);
        auto avg = INV(fs.avg);
        engine->out() << "Timings (FPS/Render Avg/Render Min/Render Max): "_k
                      << fps << "; "_k << avg << "; "_k << min << "; "_k << max
                      << "\n"_k;
    }
}

void
Anim::handleWindowResized(const ge::Event<ge::WindowResized> &ev)
{
    if (MULTISAMPLING) {
        auto w = ev.info.window.windowWidth();
        auto h = ev.info.window.windowHeight();
        engine->renderManager().setActiveRenderTarget(nullptr);
        render_texture->resize(w, h);
        engine->renderManager().setDefaultRenderTarget(render_texture);
    }
}

using ComEv = ge::Event<ge::CommandEvent>;

#define DEF_NUM_COMMAND(nm, code)                                              \
    nm(ge::makeCommand(bl::string_view_literal(PP_TOSTR(nm)),                  \
                       ""_k,                                                   \
                       [&w](const ComEv &, double x) { code; }))

Commands::Commands(World &w)
  : DEF_NUM_COMMAND(zoom, w.zoom(real(x)))
  , DEF_NUM_COMMAND(shift_x, w.shift(vec2(real(x), 0)))
  , DEF_NUM_COMMAND(shift_y, w.shift(vec2(0, real(x))))
  ,

  DEF_NUM_COMMAND(julia_constant_rotate, w.julia_constant_rotate(real(x)))
  , DEF_NUM_COMMAND(julia_constant_scale, w.julia_constant_scale(real(x)))
  , DEF_NUM_COMMAND(julia_constant_add_re,
                    w.julia_constant_add(vec2(real(x), 0)))
  , DEF_NUM_COMMAND(julia_constant_add_im,
                    w.julia_constant_add(vec2(0, real(x))))
  , julia_constant_reset(
      ge::makeCommand("julia_constant_reset"_k, ""_k, [&w](const ComEv &) {
          w.julia_constant_reset();
      }))
{}

void
Commands::registerCommands(ge::CommandProcessor &coms)
{
    coms.define(zoom);
    coms.define(shift_x);
    coms.define(shift_y);

    coms.define(julia_constant_rotate);
    coms.define(julia_constant_scale);
    coms.define(julia_constant_add_re);
    coms.define(julia_constant_add_im);
    coms.define(julia_constant_reset);
}

real
sigmoid(real t)
{
    return 1 / (1 + math::exp(t));
}

void
World::zoom(real t)
{
    scale *= 1 + 0.005_r * t;
}

void
World::shift(const vec2 &a)
{
    trans += 0.005_r * a / scale;
}

void
World::julia_constant_rotate(real phi)
{
    auto [c, s] = math::cossin(phi);
    auto x = julia_constant[0];
    auto y = julia_constant[1];
    julia_constant[0] = c * x + s * y;
    julia_constant[1] = -s * x + c * y;
}

void
World::julia_constant_scale(real t)
{
    julia_constant *= t;
}

void
World::julia_constant_add(const vec2 &z)
{
    julia_constant += z;
}

void
World::julia_constant_reset()
{
    julia_constant = vec2(0, 0);
}

int
main(int argc, char *argv[])
{
    Anim anim;
    anim.engine->setDevelDataDir(
      bl::string_view_literal(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR)));
    return anim.main(argc, argv);
}
