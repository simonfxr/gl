#include "ge/WindowRenderTarget.hpp"
#include "ge/GameWindow.hpp"
#include "glt/utils.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace ge {

namespace {

uint32_t
buffersOf(GameWindow &win)
{
	auto cs = win.contextInfo();
    auto bs = glt::RT_COLOR_BUFFER;
    if (cs.depthBits > 0)
        bs |= glt::RT_DEPTH_BUFFER;
    if (cs.stencilBits > 0)
        bs |= glt::RT_STENCIL_BUFFER;
    return bs;
}

} // namespace

WindowRenderTarget::WindowRenderTarget(GameWindow &w)
  : RenderTarget(w.windowWidth(), w.windowHeight(), buffersOf(w)), window(w)
{}

void
WindowRenderTarget::resized()
{
    updateSize(window.windowWidth(), window.windowHeight());
}

void
WindowRenderTarget::doActivate()
{
    GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void
WindowRenderTarget::doDraw()
{
    window.swapBuffers();
    if (window.vsync())
        GL_CALL(glFinish);
}

} // namespace ge
