#include "ge/WindowRenderTarget.hpp"
#include "ge/GameWindow.hpp"
#include "glt/utils.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace ge {

namespace {

uint32 buffersOf(GameWindow& win) {
    const sf::ContextSettings& cs = win.window().getSettings();
    uint32 bs = glt::RT_COLOR_BUFFER;
    if (cs.depthBits > 0)
        bs |= glt::RT_DEPTH_BUFFER;
    if (cs.stencilBits > 0)
        bs |= glt::RT_STENCIL_BUFFER;
    return bs;
}

} // namespace anon

WindowRenderTarget::WindowRenderTarget(GameWindow& w) :
    RenderTarget(SIZE(w.window().getSize().x), SIZE(w.window().getSize().y),
                 buffersOf(w)),
    window(w)
{}

void WindowRenderTarget::resized() {
    updateSize(SIZE(window.window().getSize().x), SIZE(window.window().getSize().y));
}

void WindowRenderTarget::doActivate() {
    window.window().setActive();
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void WindowRenderTarget::doDraw() {
    window.window().display();
    if (window.vsync())
        GL_CHECK(glFinish());
}

} // namespace ge

