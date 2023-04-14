#include "ge/Anim.hpp"

#include "math/fvec.hpp"
#include "utils.hpp"

using namespace math;

struct Sim
{
    real m1, m2;
    real l;
    real g;

    real x1;
    real dx1_dt;
    real phi;
    real dphi_dt;

    [[nodiscard]] real acc_x1(real x1,
                              real dx_dt,
                              real alpha,
                              real dalpha_dt) const;

    [[nodiscard]] real acc_phi(real x1,
                               real dx_dt,
                               real alpha,
                               real dalpha_dt) const;

    void init();
    void integrate(real dt);
};

struct Anim final : ge::Anim<Anim>
{
    ParticleRenderer renderer;
    Sim simulation{};

    void init(const ge::Event<ge::InitEvent> &);

    void render(const ge::Event<ge::RenderEvent> &) override;
    void animate(const ge::Event<ge::AnimationEvent> &) override;
};

void
Anim::init(const ge::Event<ge::InitEvent> &ev)
{
    engine.gameLoop().ticks(1000);
    engine.gameLoop().maxFPS(59);

    engine.window().showMouseCursor(true);

    simulation.init();

    ParticleRenderer::Opts propts;
    propts.world_size = vec2(1, 1);
    if (!renderer.init(&engine, propts))
        return;

    ev.info.success = true;
}

void
Anim::render(const ge::Event<ge::RenderEvent> &ev)
{
    auto rt = engine.renderManager().activeRenderTarget();
    rt->clearColor(glt::color(vec4(1_r)));
    rt->clear();

    ParticleRenderer::Particle p1{}, p2{};
    p1.radius = 0.05f;
    p2.radius = 0.05f;

    p1.color = p2.color = vec4(1, 0, 0, 1);
    p1.position = vec2(simulation.x1, 0);

    auto [cos_phi, sin_phi] = cossin(simulation.phi);
    p2.position =
      vec2(simulation.x1 + simulation.l * sin_phi, -simulation.l * cos_phi);

    ParticleRenderer::Particle ps[] = { p1, p2 };
    renderer.renderParticles(ev, ARRAY_LENGTH(ps), ps);
}

real
Sim::acc_x1(real x, real dx_dt, real alpha, real dalpha_dt) const
{
    UNUSED(x);
    UNUSED(dx_dt);
    auto [cos_alpha, sin_alpha] = cossin(alpha);
    return (l * m2 * sin_alpha * squared(dalpha_dt) +
            g * m2 * cos_alpha * sin_alpha) /
           (m2 * squared(sin_alpha) + m1);
}

real
Sim::acc_phi(real x, real dx_dt, real alpha, real dalpha_dt) const
{
    UNUSED(x);
    UNUSED(dx_dt);
    auto [cos_alpha, sin_alpha] = cossin(alpha);
    return -(l * m2 * cos_alpha * sin_alpha * squared(dalpha_dt) +
             g * (m1 + m2) * sin_alpha) /
           (l * m2 * squared(sin_alpha) + l * m1);
}

void
Sim::integrate(real dt)
{

    auto dt2 = dt * 0.5_r;

    auto a1_x1 = acc_x1(x1, dx1_dt, phi, dphi_dt);
    auto a1_phi = acc_phi(x1, dx1_dt, phi, dphi_dt);

    auto a2_x1 = acc_x1(x1 + dx1_dt * dt2,
                        dx1_dt + a1_x1 * dt2,
                        phi + dphi_dt * dt2,
                        dphi_dt + a1_phi * dt2);
    auto a2_phi = acc_phi(x1 + dx1_dt * dt2,
                          dx1_dt + a1_x1 * dt2,
                          phi + dphi_dt * dt2,
                          dphi_dt + a1_phi * dt2);

    auto a3_x1 = acc_x1(x1 + dx1_dt * dt2,
                        dx1_dt + a2_x1 * dt2,
                        phi + dphi_dt * dt2,
                        dphi_dt + a2_phi * dt2);
    auto a3_phi = acc_phi(x1 + dx1_dt * dt2,
                          dx1_dt + a2_x1 * dt2,
                          phi + dphi_dt * dt2,
                          dphi_dt + a2_phi * dt2);

    auto P2 = [](real a, real b, real c, real xdt) {
        return a + (b + 0.5_r * c * xdt) * xdt;
    };

    auto a4_x1 = acc_x1(P2(x1, dx1_dt, a3_x1, dt),
                        dx1_dt + a3_x1 * dt,
                        P2(phi, dphi_dt, a3_phi, dt),
                        dphi_dt + a3_phi * dt);
    auto a4_phi = acc_phi(P2(x1, dx1_dt, a3_x1, dt),
                          dx1_dt + a3_x1 * dt,
                          P2(phi, dphi_dt, a3_phi, dt),
                          dphi_dt + a3_phi * dt);

    auto a_x1 = (a1_x1 + 2 * (a2_x1 + a3_x1) + a4_x1) / 6.f;
    auto a_phi = (a1_phi + 2 * (a2_phi + a3_phi) + a4_phi) / 6.f;

    x1 = P2(x1, dx1_dt, a_x1, dt);
    dx1_dt += a_x1 * dt;

    phi = P2(phi, dphi_dt, a_phi, dt);
    dphi_dt += a_phi * dt;
}

void
Sim::init()
{
    g = 9.81f;
    m1 = 1;

    m2 = 0.5f;
    l = 0.5;
    x1 = 0;
    dx1_dt = 0;
    phi = 1;
    dphi_dt = 0;
};

void
Anim::animate(const ge::Event<ge::AnimationEvent> &ev)
{
    auto dt =
      0.15_r * static_cast<real>(ev.info.engine.gameLoop().tickDuration());
    simulation.integrate(dt);
};

int
main(int argc, char *argv[])
{
    Anim anim;
    anim.engine.setDevelDataDir(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR));
    return anim.main(argc, argv);
}
