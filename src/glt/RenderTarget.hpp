#ifndef GLT_RENDER_TARGET_HPP
#define GLT_RENDER_TARGET_HPP

#include "glt/color.hpp"
#include "glt/conf.hpp"

#include <memory>

namespace glt {

using namespace defs;

struct GLT_API Viewport
{
    int32 offsetX, offsetY;
    size width, height;

    Viewport() : offsetX(0), offsetY(0), width(0), height(0) {}

    Viewport(int32 x, int32 y, size w, size h)
      : offsetX(x), offsetY(y), width(w), height(h)
    {}

    Viewport(size w, size h) : offsetX(0), offsetY(0), width(w), height(h) {}

    bool operator==(const Viewport &vp) const;
    bool operator!=(const Viewport &vp) const;
};

typedef uint32 RenderTargetBuffers;

static const RenderTargetBuffers RT_COLOR_BUFFER = 1u;
static const RenderTargetBuffers RT_DEPTH_BUFFER = 2u;
static const RenderTargetBuffers RT_STENCIL_BUFFER = 4u;

static const uint32 RT_ALL_BUFFERS =
  RT_COLOR_BUFFER | RT_DEPTH_BUFFER | RT_STENCIL_BUFFER;

struct GLT_API RenderTarget
{

    RenderTarget(size width,
                 size height,
                 RenderTargetBuffers bs = RT_COLOR_BUFFER,
                 const Viewport &vp = Viewport());
    virtual ~RenderTarget();

    size width() const;
    size height() const;
    RenderTargetBuffers buffers() const;
    const Viewport &viewport() const;
    color clearColor() const;
    void clearColor(const color &);

    void activate();
    void deactivate();
    void beginScene();
    void clear(RenderTargetBuffers buffers = RT_ALL_BUFFERS);
    void draw();
    void viewport(const Viewport &vp);

protected:
    void updateSize(size width, size height);

    virtual void doActivate() = 0;
    virtual void doDeactivate();
    virtual void doClear(RenderTargetBuffers buffers, color col);
    virtual void doDraw();
    virtual void doViewport(const Viewport &vp);

private:
    DECLARE_PIMPL(self);
};

inline bool
Viewport::operator==(const Viewport &vp) const
{
    return this->offsetX == vp.offsetX && this->offsetY == vp.offsetY &&
           this->width == vp.width && this->height == vp.height;
}

inline bool
Viewport::operator!=(const Viewport &vp) const
{
    return !(*this == vp);
}

} // namespace glt

#endif
