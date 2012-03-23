#include "ge/Engine.hpp"

using namespace defs;
using namespace math;

struct Anim {
    ge::Engine engine;
        
    void init(const ge::Event<ge::InitEvent>&);
    void link();

    void renderScene(const ge::Event<ge::RenderEvent>&);
};

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    engine.window().showMouseCursor(true);
    ev.info.success = true;
}

void Anim::link() {
    engine.events().render.reg(ge::makeEventHandler(this, &Anim::renderScene));
}

void Anim::renderScene(const ge::Event<ge::RenderEvent>&) {
    glt::RenderTarget *rt = engine.renderManager().activeRenderTarget();
    rt->clearColor(glt::color(vec4(real(1))));
    rt->clear();
}

int main(int argc, char *argv[]) {
    ge::EngineOptions opts;
    Anim anim;
    anim.link();
    opts.parse(&argc, &argv);
    return anim.engine.run(opts);
}