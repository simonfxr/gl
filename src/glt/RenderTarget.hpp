#ifndef GLT_RENDER_TARGET_HPP
#define GLT_RENDER_TARGET_HPP

#include "defs.h"

namespace glt {

struct Viewport {
    uint32 offsetX, offsetY;
    uint32 width, height;

    Viewport() :
        offsetX(0), offsetY(0),
        width(0), height(0) {}

    Viewport(uint32 x, uint32 y, uint32 w, uint32 h) :
        offsetX(x), offsetY(y),
        width(w), height(h) {}

    Viewport(uint32 w, uint32 h) :
        offsetX(0), offsetY(0),
        width(w), height(h) {}

    bool operator ==(const Viewport& vp) const;
    bool operator !=(const Viewport& vp) const;
};

typedef uint32 RenderTargetBuffers;

static const RenderTargetBuffers RT_COLOR_BUFFER = 1u;
static const RenderTargetBuffers RT_DEPTH_BUFFER = 2u;
static const RenderTargetBuffers RT_STENCIL_BUFFER = 4u;

static const uint32 RT_ALL_BUFFERS = RT_COLOR_BUFFER | RT_DEPTH_BUFFER | RT_STENCIL_BUFFER;

struct RenderTarget {

    RenderTarget(uint32 width, uint32 height, RenderTargetBuffers buffers = RT_COLOR_BUFFER, const Viewport& vp = Viewport());
    virtual ~RenderTarget();

    uint32 width() const;
    uint32 height() const;
    uint32 buffers() const;
    const Viewport& viewport() const;

    void activate();
    void deactivate();
    void beginScene();
    void clear(RenderTargetBuffers buffers = RT_ALL_BUFFERS);
    void draw();
    void viewport(const Viewport& vp);

protected:

    void updateSize(uint32 width, uint32 height);

    virtual void doActivate() = 0;
    virtual void doDeactivate();
    virtual void doClear(RenderTargetBuffers buffers);
    virtual void doDraw();
    virtual void doViewport(const Viewport& vp);

private:
    struct Data;
    Data * const self;

    RenderTarget(const RenderTarget& _);
    RenderTarget& operator =(const RenderTarget& _);
};

inline bool Viewport::operator ==(const Viewport& vp) const {
    return this->offsetX == vp.offsetX &&
           this->offsetY == vp.offsetY &&
           this->width == vp.width &&
           this->height == vp.height;
}

inline bool Viewport::operator !=(const Viewport& vp) const {
    return !(*this == vp);
}

} // namespace glt

#endif
