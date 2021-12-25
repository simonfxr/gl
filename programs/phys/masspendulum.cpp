#include "ge/Engine.hpp"

#include "math/vec4.hpp"
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

    real acc_x1(real x1, real dx1_dt, real phi, real dphi_dt);
    real acc_phi(real x1, real dx1_dt, real phi, real dphi_dt);

    void init();
    void integrate(real dt);
};

struct Anim
{
    ge::Engine engine;
    ParticleRenderer renderer;
    Sim simulation;

    void init(const ge::Event<ge::InitEvent> &);
    void link();

    void renderScene(const ge::Event<ge::RenderEvent> &);
    void simulate(const ge::Event<ge::AnimationEvent> &);
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
Anim::link()
{
    engine.events().render.reg(*this, &Anim::renderScene);
    engine.events().animate.reg(*this, &Anim::simulate);
}

void
Anim::renderScene(const ge::Event<ge::RenderEvent> &ev)
{
    auto rt = engine.renderManager().activeRenderTarget();
    rt->clearColor(glt::color(vec4(real(1))));
    rt->clear();

    ParticleRenderer::Particle p1, p2;
    p1.radius = 0.05f;
    p2.radius = 0.05f;

    p1.color = p2.color = vec4(1, 0, 0, 1);
    p1.position = vec2(simulation.x1, 0);

    real sin_phi, cos_phi;
    sincos(simulation.phi, sin_phi, cos_phi);
    p2.position =
      vec2(simulation.x1 + simulation.l * sin_phi, -simulation.l * cos_phi);

    ParticleRenderer::Particle ps[] = { p1, p2 };
    renderer.renderParticles(ev, ARRAY_LENGTH(ps), ps);
}

real
Sim::acc_x1(real x, real dx_dt, real alpha, real dalpha_dt)
{
    UNUSED(x);
    UNUSED(dx_dt);
    real sin_alpha, cos_alpha;
    sincos(alpha, sin_alpha, cos_alpha);
    return (l * m2 * sin_alpha * squared(dalpha_dt) +
            g * m2 * cos_alpha * sin_alpha) /
           (m2 * squared(sin_alpha) + m1);
}

real
Sim::acc_phi(real x, real dx_dt, real alpha, real dalpha_dt)
{
    UNUSED(x);
    UNUSED(dx_dt);
    real sin_alpha, cos_alpha;
    sincos(alpha, sin_alpha, cos_alpha);
    return -(l * m2 * cos_alpha * sin_alpha * squared(dalpha_dt) +
             g * (m1 + m2) * sin_alpha) /
           (l * m2 * squared(sin_alpha) + l * m1);
}

void
Sim::integrate(real dt)
{

    real dt2 = dt * 0.5f;

    real a1_x1 = acc_x1(x1, dx1_dt, phi, dphi_dt);
    real a1_phi = acc_phi(x1, dx1_dt, phi, dphi_dt);

    real a2_x1 = acc_x1(x1 + dx1_dt * dt2,
                        dx1_dt + a1_x1 * dt2,
                        phi + dphi_dt * dt2,
                        dphi_dt + a1_phi * dt2);
    real a2_phi = acc_phi(x1 + dx1_dt * dt2,
                          dx1_dt + a1_x1 * dt2,
                          phi + dphi_dt * dt2,
                          dphi_dt + a1_phi * dt2);

    real a3_x1 = acc_x1(x1 + dx1_dt * dt2,
                        dx1_dt + a2_x1 * dt2,
                        phi + dphi_dt * dt2,
                        dphi_dt + a2_phi * dt2);
    real a3_phi = acc_phi(x1 + dx1_dt * dt2,
                          dx1_dt + a2_x1 * dt2,
                          phi + dphi_dt * dt2,
                          dphi_dt + a2_phi * dt2);

#define P2(a, b, c, dt) (a + (b + 0.5f * c * dt) * dt)

    real a4_x1 = acc_x1(P2(x1, dx1_dt, a3_x1, dt),
                        dx1_dt + a3_x1 * dt,
                        P2(phi, dphi_dt, a3_phi, dt),
                        dphi_dt + a3_phi * dt);
    real a4_phi = acc_phi(P2(x1, dx1_dt, a3_x1, dt),
                          dx1_dt + a3_x1 * dt,
                          P2(phi, dphi_dt, a3_phi, dt),
                          dphi_dt + a3_phi * dt);

    real a_x1 = (a1_x1 + 2 * (a2_x1 + a3_x1) + a4_x1) / 6.f;
    real a_phi = (a1_phi + 2 * (a2_phi + a3_phi) + a4_phi) / 6.f;

    x1 = P2(x1, dx1_dt, a_x1, dt);
    dx1_dt += a_x1 * dt;

    phi = P2(phi, dphi_dt, a_phi, dt);
    dphi_dt += a_phi * dt;

#undef P2
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
Anim::simulate(const ge::Event<ge::AnimationEvent> &ev)
{
    real dt = 0.15_r * math::real(ev.info.engine.gameLoop().tickDuration());
    simulation.integrate(dt);
};

int
main(int argc, char *argv[])
{
    ge::EngineOptions opts;
    Anim anim;
    anim.engine.setDevelDataDir(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR));
    anim.link();
    opts.inits.reg(ge::Init, anim, &Anim::init);
    opts.parse(&argc, &argv);
    return anim.engine.run(opts);
}
