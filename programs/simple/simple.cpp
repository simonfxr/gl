#include "ge/Engine.hpp"

using namespace math;

struct Anim
{
    ge::Engine engine;

    void init(const ge::Event<ge::InitEvent> & /*ev*/);
    void link();

    void renderScene(const ge::Event<ge::RenderEvent> & /*unused*/);
};

void
Anim::init(const ge::Event<ge::InitEvent> &ev)
{
    engine.window().showMouseCursor(true);
    ev.info.success = true;
}

void
Anim::link()
{
    engine.events().render.reg(ge::makeEventHandler(this, &Anim::renderScene));
}

void
Anim::renderScene(const ge::Event<ge::RenderEvent> & /*unused*/)
{
    glt::RenderTarget *rt = engine.renderManager().activeRenderTarget();
    rt->clearColor(glt::color(vec4(real(1))));
    rt->clear();
}

int
main(int argc, char *argv[])
{
    ge::EngineOptions opts;
    Anim anim;
    anim.engine.setDevelDataDir(CMAKE_CURRENT_SOURCE_DIR);
    anim.link();
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));
    opts.parse(&argc, &argv);
    return anim.engine.run(opts);
}
