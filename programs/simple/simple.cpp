#include "ge/Anim.hpp"

using namespace math;

struct Anim final : ge::Anim<Anim>
{
    void init(const ge::Event<ge::InitEvent> & /*ev*/);
    void render(const ge::Event<ge::RenderEvent> & /*unused*/) override;
};

void
Anim::init(const ge::Event<ge::InitEvent> &ev)
{
    engine->window().showMouseCursor(true);
    ev.info.success = true;
}

void
Anim::render(const ge::Event<ge::RenderEvent> & /*unused*/)
{
    auto rt = engine->renderManager().activeRenderTarget();
    rt->clearColor(glt::color(vec4(1_r)));
    rt->clear();
}

int
main(int argc, char *argv[])
{
    Anim anim;
    anim.engine->setDevelDataDir(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR));
    return anim.main(argc, argv);
}
