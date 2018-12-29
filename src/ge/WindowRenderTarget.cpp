#include "ge/WindowRenderTarget.hpp"
#include "ge/GameWindow.hpp"
#include "glt/utils.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace ge {

namespace {

uint32
buffersOf(GameWindow &win)
{
    GLContextInfo cs;
    win.contextInfo(cs);
    uint32 bs = glt::RT_COLOR_BUFFER;
    if (cs.depthBits > 0)
        bs |= glt::RT_DEPTH_BUFFER;
    if (cs.stencilBits > 0)
        bs |= glt::RT_STENCIL_BUFFER;
    return bs;
}

} // namespace

WindowRenderTarget::WindowRenderTarget(GameWindow &w)
  : RenderTarget(SIZE(w.windowWidth()), SIZE(w.windowHeight()), buffersOf(w))
  , window(w)
{}

void
WindowRenderTarget::resized()
{
    updateSize(SIZE(window.windowWidth()), SIZE(window.windowHeight()));
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
