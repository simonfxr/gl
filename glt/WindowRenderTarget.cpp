#include "glt/WindowRenderTarget.hpp"
#include "glt/utils.hpp"
#include "math/vec4.hpp"

namespace glt {

namespace {

uint32 buffersOf(ge::GameWindow& win) {
    const sf::ContextSettings& cs = win.window().GetSettings();
    uint32 bs = RT_COLOR_BUFFER;
    if (cs.DepthBits > 0)
        bs |= RT_DEPTH_BUFFER;
    if (cs.StencilBits > 0)
        bs |= RT_STENCIL_BUFFER;
    return bs;
}

} // namespace anon

WindowRenderTarget::WindowRenderTarget(ge::GameWindow& w) :
    RenderTarget(w.window().GetWidth(), w.window().GetHeight(),
                 buffersOf(w)),
    window(w)
{}

void WindowRenderTarget::doActivate() {
    window.window().SetActive();
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void WindowRenderTarget::doDeactivate() {
    window.window().SetActive(false);
}

void WindowRenderTarget::doDraw() {
    window.window().Display();
}

void WindowRenderTarget::doViewport(const Viewport& vp) {
    updateSize(window.window().GetWidth(), window.window().GetHeight());
    RenderTarget::doViewport(vp);
}

} // namespace glt

