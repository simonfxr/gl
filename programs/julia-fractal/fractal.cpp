#include "ge/Engine.hpp"


#include "math/real.hpp"
#include "math/vec2.hpp"
#include "math/glvec.hpp"

#include "glt/ShaderProgram.hpp"
#include "glt/Mesh.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/TextureRenderTarget.hpp"
#include "glt/utils.hpp"

#include "shaders/quasi_constants.h"

#include "ge/Engine.hpp"
#include "ge/Command.hpp"
#include "ge/Event.hpp"

static const bool MULTISAMPLING = false; // via multisample texture

using namespace math;

#define VERTEX(V, F, Z) \
    V(Vertex, Z(vec2_t, position))

DEFINE_VERTEX(VERTEX);
#undef VERTEX

struct World {
    real scale = 1;
    vec2_t trans = vec2(0, 0);
    vec2_t julia_constant = vec2(0.3, 0.5);

    void zoom(real t);
    void shift(const vec2_t&);

    void julia_constant_rotate(real phi);
    void julia_constant_scale(real s);
    void julia_constant_add(const vec2_t&);
    void julia_constant_reset();
};

struct Commands : public ge::Plugin {

    Ref<ge::Command> zoom;
    Ref<ge::Command> shift_x;
    Ref<ge::Command> shift_y;
    
    Ref<ge::Command> julia_constant_rotate;
    Ref<ge::Command> julia_constant_scale;
    Ref<ge::Command> julia_constant_add_re;
    Ref<ge::Command> julia_constant_add_im;
    Ref<ge::Command> julia_constant_reset;

    Commands(World&);

    void registerWith(ge::Engine&) final override;
    void registerCommands(ge::CommandProcessor&) final override;
};

struct Anim {
    ge::Engine engine;
    glt::CubeMesh<Vertex> quadBatch;
    Ref<glt::TextureRenderTarget> render_texture;

    World world;
    Ref<Commands> commands;

    real time_print_fps;
    
    void init(const ge::Event<ge::InitEvent>&);
    void link(ge::Engine& e);
    void animate(const ge::Event<ge::AnimationEvent>&);
    void renderScene(const ge::Event<ge::RenderEvent>&);
    
    void handleWindowResized(const ge::Event<ge::WindowResized>& ev);
};

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    link(ev.info.engine);

    commands = makeRef(new Commands(world));
    engine.enablePlugin(*commands);

    time_print_fps = 0;
    
    {
        Vertex v;
        v.position = vec2(-1.f, -1.f); quadBatch.add(v);
        v.position = vec2( 1.f, -1.f); quadBatch.add(v);
        v.position = vec2( 1.f,  1.f); quadBatch.add(v);
        v.position = vec2(-1.f,  1.f); quadBatch.add(v);

        quadBatch.send();
    }

    if (MULTISAMPLING) {
        size w = engine.window().windowWidth();
        size h = engine.window().windowHeight();
        glt::TextureRenderTarget::Params ps;
        ps.samples = NUM_SAMPLES;
        ps.buffers = glt::RT_COLOR_BUFFER | glt::RT_DEPTH_BUFFER;
        render_texture = makeRef(new glt::TextureRenderTarget(w, h, ps));
        engine.renderManager().setDefaultRenderTarget(render_texture.ptr());

        GL_CALL(glEnable, GL_MULTISAMPLE);
    }

    GL_CALL(glDisable, GL_DEPTH_TEST);

    ev.info.success = true;

    
}

void Anim::link(ge::Engine& e) {
    e.events().animate.reg(ge::makeEventHandler(this, &Anim::animate));
    e.events().render.reg(ge::makeEventHandler(this, &Anim::renderScene));
    e.window().events().windowResized.reg(ge::makeEventHandler(this, &Anim::handleWindowResized));
}

void Anim::animate(const ge::Event<ge::AnimationEvent>&) {
    // empty
}

void Anim::renderScene(const ge::Event<ge::RenderEvent>& ev) {
    ge::Engine& e = ev.info.engine;
    real time = e.gameLoop().tickTime() + ev.info.interpolation * e.gameLoop().tickDuration();

    glt::RenderManager& rm = engine.renderManager();

    if (MULTISAMPLING) {
        rm.setActiveRenderTarget(render_texture.ptr());
        render_texture->clear();
    }

    Ref<glt::ShaderProgram> renderShader = e.shaderManager().program("render");
    ASSERT(renderShader);

    renderShader->use();
    glt::Uniforms(*renderShader)
        .optional("gammaCorrection", real(2.2))
        .optional("time", time)
        .optional("transform", mat3())
        .optional("julia_constant", world.julia_constant)
        .optional("world_shift", world.trans)
        .optional("world_zoom", world.scale);

    quadBatch.draw();

    if (MULTISAMPLING) {
        rm.setActiveRenderTarget(&engine.window().renderTarget());
        Ref<glt::ShaderProgram> postproc = e.shaderManager().program("postproc");
        ASSERT(postproc);

        postproc->use();

        render_texture->sampler().bind(0);
        glt::Uniforms(*postproc)
            .optional("texture0", glt::Sampler(render_texture->sampler(), 0));
        quadBatch.draw();
    }

    if (time >= time_print_fps) {
        time_print_fps = time + 1;

        #define INV(x) (((x) * (x)) <= 0 ? -1 : 1.0 / (x))
        glt::FrameStatistics fs = engine.renderManager().frameStatistics();
        double fps = INV(fs.avg);
        double min = INV(fs.max);
        double max = INV(fs.min);
        double avg = INV(fs.avg);
        engine.out() << "Timings (FPS/Render Avg/Render Min/Render Max): " << fps << "; " << avg << "; " << min << "; " << max << sys::io::endl;
    }
}

void Anim::handleWindowResized(const ge::Event<ge::WindowResized>& ev) {
    if (MULTISAMPLING) {
        size w = ev.info.window.windowWidth();
        size h = ev.info.window.windowHeight();
        engine.renderManager().setActiveRenderTarget(0);
        render_texture->resize(w, h);
        engine.renderManager().setDefaultRenderTarget(render_texture.ptr());
    }
}

typedef ge::Event<ge::CommandEvent> ComEv;
typedef Array<ge::CommandArg> ComArgs;

#define DEF_NUM_COMMAND(nm, code) \
    nm(ge::makeNumCommand([&w](const ComEv&, double x) { code; }, #nm, ""))

Commands::Commands(World& w) :
    DEF_NUM_COMMAND(zoom, w.zoom(real(x))),
    DEF_NUM_COMMAND(shift_x, w.shift(vec2(real(x), 0))),
    DEF_NUM_COMMAND(shift_y, w.shift(vec2(0, real(x)))),
    
    DEF_NUM_COMMAND(julia_constant_rotate, w.julia_constant_rotate(real(x))),
    DEF_NUM_COMMAND(julia_constant_scale, w.julia_constant_scale(real(x))),
    DEF_NUM_COMMAND(julia_constant_add_re, w.julia_constant_add(vec2(real(x), 0))),
    DEF_NUM_COMMAND(julia_constant_add_im, w.julia_constant_add(vec2(0, real(x)))),
    julia_constant_reset(makeCommand([&w](const ComEv&, const ComArgs&) { w.julia_constant_reset(); },
                                     ge::NULL_PARAMS,
                                     "julia_constant_reset", ""))
{}

void Commands::registerWith(ge::Engine&) {
    // NOOP
}

void Commands::registerCommands(ge::CommandProcessor& coms) {
    coms.define(zoom);
    coms.define(shift_x);
    coms.define(shift_y);
    
    coms.define(julia_constant_rotate);
    coms.define(julia_constant_scale);
    coms.define(julia_constant_add_re);
    coms.define(julia_constant_add_im);
    coms.define(julia_constant_reset);
}

real sigmoid(real t) {
    return 1/(1 + exp(t));
}

void World::zoom(real t) {
    scale *= 1 + 0.005 * t;
}

void World::shift(const vec2_t& a) {
    trans += 0.005 * a / scale;
}

void World::julia_constant_rotate(real phi) {
    real s, c;
    math::sincos(phi, s, c);
    real x = julia_constant[0];
    real y = julia_constant[1];
    julia_constant[0] = c * x + s * y;
    julia_constant[0] = -s * x + c * y;
}

void World::julia_constant_scale(real t) {
    julia_constant *= t;
}

void World::julia_constant_add(const vec2_t& z) {
    julia_constant += z;
}

void World::julia_constant_reset() {
    julia_constant = vec2(0, 0);
}

int main(int argc, char *argv[]) {
    Anim anim;
    ge::EngineOptions opts;

    opts.parse(&argc, &argv);
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));
    return anim.engine.run(opts);
}
