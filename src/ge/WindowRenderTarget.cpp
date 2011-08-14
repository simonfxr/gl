#include "ge/WindowRenderTarget.hpp"
#include "ge/GameWindow.hpp"
#include "glt/utils.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace ge {

namespace {

uint32 buffersOf(GameWindow& win) {
    const sf::ContextSettings& cs = win.window().GetSettings();
    uint32 bs = glt::RT_COLOR_BUFFER;
    if (cs.DepthBits > 0)
        bs |= glt::RT_DEPTH_BUFFER;
    if (cs.StencilBits > 0)
        bs |= glt::RT_STENCIL_BUFFER;
    return bs;
}

} // namespace anon

WindowRenderTarget::WindowRenderTarget(GameWindow& w) :
    RenderTarget(w.window().GetWidth(), w.window().GetHeight(),
                 buffersOf(w)),
    window(w)
{}

void WindowRenderTarget::resized() {
    updateSize(window.window().GetWidth(), window.window().GetHeight());
}

void WindowRenderTarget::doActivate() {
    window.window().SetActive();
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void WindowRenderTarget::doDraw() {
    window.window().Display();
}

} // namespace ge

