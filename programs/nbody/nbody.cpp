#include "ge/Camera.hpp"
#include "ge/Engine.hpp"
#include "ge/MouseLookPlugin.hpp"
#include "glt/Mesh.hpp"
#include "glt/Transformations.hpp"
#include "glt/primitives.hpp"
#include "glt/utils.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

#include <algorithm>
#include <map>
#include <vector>

#include "nbody/nbody_phys.hpp"

using namespace math;

static const size_t SIMULATION_FPS = 50;

#define USE_NO_SIM

#ifdef USE_NO_SIM
#    define SIM_CALL(...)
#else
#    define SIM_CALL(...) __VA_ARGS__
#endif

DEF_GL_MAPPED_TYPE(Vertex, (vec3_t, position), (vec3_t, normal))

struct Anim
{
    ge::Engine engine;
    glt::Mesh<Vertex> sphere_model;
    glt::CubeMesh<Vertex> ground_model;
    ge::Camera camera;
    ge::MouseLookPlugin mouse_look;

    vec3_t light_dir{};
    vec3_t ec_light_dir{};

    ge::GameLoop::time next_fps_counter_draw{};

    Simulation sim;

    Anim() : sim(1_r / static_cast<real>(SIMULATION_FPS)) {}

    void init(const ge::Event<ge::InitEvent> &);
    void link();

    void renderScene(const ge::Event<ge::RenderEvent> &);
    void animate(const ge::Event<ge::AnimationEvent> &);

    void renderParticles(real interpolation);
};

void
Anim::init(const ge::Event<ge::InitEvent> &ev)
{

    ev.info.engine.gameLoop().ticks(SIMULATION_FPS)
      //        .pause(true)
      ;

    SIM_CALL(sim.init());

    next_fps_counter_draw = 0.3_r;

    glt::primitives::sphere(sphere_model, 1.f, 36, 24);
    sphere_model.send();

    {
        auto vert = Vertex{};
        auto d2 = .5_r;
        vert.normal = vec3(0, 1, 0);
        vert.position = vec3(-d2, 0, d2);
        ground_model.add(vert);
        vert.position = vec3(d2, 0, d2);
        ground_model.add(vert);
        vert.position = vec3(d2, 0, -d2);
        ground_model.add(vert);
        vert.position = vec3(-d2, 0, -d2);
        ground_model.add(vert);
        ground_model.send();
    }

    GL_CALL(glEnable, GL_DEPTH_TEST);
    GL_CALL(glEnable, GL_MULTISAMPLE);

    light_dir = normalize(vec3(-1_r));

    ev.info.engine.enablePlugin(mouse_look);
    mouse_look.camera(&camera);
    ev.info.engine.enablePlugin(camera);
    camera.frame().origin = vec3(0_r);

    ev.info.success = true;
}

void
Anim::link()
{
    engine.events().render.reg(*this, &Anim::renderScene);
    engine.events().animate.reg(*this, &Anim::animate);
}

void
Anim::animate(const ge::Event<ge::AnimationEvent> &)
{
    UNUSED(this);
    SIM_CALL(sim.simulate_frame());
}

void
Anim::renderScene(const ge::Event<ge::RenderEvent> &ev)
{
    auto rt = engine.renderManager().activeRenderTarget();
    rt->clearColor(glt::color(vec4(1_r)));
    rt->clear();

    glt::RenderManager &rm = engine.renderManager();
    ec_light_dir = normalize(
      transformVector(rm.geometryTransform().viewMatrix(), light_dir));

    {
        auto program = engine.shaderManager().program("ground");
        ASSERT(program, "ground program not found");

        rm.geometryTransform().dup();
        rm.geometryTransform().scale(vec3(20_r));

        program->use();

        glt::Uniforms(*program)
          .optional("mvpMatrix", rm.geometryTransform().mvpMatrix())
          //            .optional("mvMatrix", rm.geometryTransform().mvMatrix())
          .optional("normalMatrix", rm.geometryTransform().normalMatrix())
          .optional("ecLightDir", ec_light_dir)
          .optional("albedo", vec3(0.5_r));
        ground_model.draw();

        rm.geometryTransform().pop();
    }

    renderParticles(ev.info.interpolation);

    if (engine.gameLoop().realTime() > next_fps_counter_draw) {
        next_fps_counter_draw = engine.gameLoop().realTime() + 3_r;

#define INV(x) (((x) * (x)) <= 0 ? -1. : 1.0 / (x))
        auto fs = engine.renderManager().frameStatistics();
        auto fps = INV(fs.avg);
        auto min = INV(fs.max);
        auto max = INV(fs.min);
        auto avg = INV(fs.avg);
        sys::io::stderr() << "Timings (FPS/Render Avg/Render Min/Render Max): "
                          << fps << "; " << avg << "; " << min << "; " << max
                          << "\n";
#undef INV
    }
}

void
Anim::renderParticles(real interpolation)
{
    auto &rm = engine.renderManager();
#ifndef USE_NO_SIM
    auto &gt = rm.geometryTransform();

    auto program = engine.shaderManager().program("particle");
    ASSERT(program, "particle program not found");

    program->use();

    gt.dup();

    for (defs::index i = 0; i < sim.particles.size(); ++i) {
        Particle p = sim.particles[i];
        sim.extrapolate_particle(p, interpolation);

        gt.dup();
        gt.translate(p.position);
        gt.scale(vec3(real(p.radius)));

        glt::Uniforms(*program)
          .optional("mvpMatrix", gt.mvpMatrix())
          .optional("normalMatrix", gt.normalMatrix())
          .optional("ecLightDir", ec_light_dir)
          .optional("albedo", vec3(real(1), real(0), real(0)));

        gt.pop();

        sphere_model.draw();
    }

    gt.pop();
#else
    UNUSED(interpolation);
    UNUSED(rm);
#endif
}

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
