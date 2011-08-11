#include "glt/RenderTarget.hpp"
#include "error/error.hpp"
#include "glt/utils.hpp"
#include "opengl.h"

namespace glt {

struct RenderTarget::Data {
    uint32 width;
    uint32 height;
    RenderTargetBuffers buffers;
    Viewport viewport;

    DEBUG_DECL(bool active;)

    bool viewport_changed;

    Data(uint32 w, uint32 h, RenderTargetBuffers bs, const Viewport& vp) :
        width(w), height(h), buffers(bs),
        viewport(vp),
        viewport_changed(false)
        {
            ON_DEBUG(active = false);
        }

    Viewport effectiveViewport() const {
        return viewport == Viewport() ? Viewport(width, height) : viewport;
    }
};

RenderTarget::RenderTarget(uint32 w, uint32 h, uint32 bs, const Viewport& vp) :
    self(new Data(w, h, bs, vp))
{}

RenderTarget::~RenderTarget() {
    DEBUG_ASSERT(!self->active);
    delete self;
}

uint32 RenderTarget::width() const {
    return self->width;
}

uint32 RenderTarget::height() const {
    return self->height;
}

uint32 RenderTarget::buffers() const {
    return self->buffers;
}

const Viewport& RenderTarget::viewport() const {
    return self->viewport;
}

void RenderTarget::activate() {
    DEBUG_ASSERT(!self->active);
    ON_DEBUG(self->active = true);
    self->viewport_changed = false;
    doActivate();
    doViewport(self->effectiveViewport());
}

void RenderTarget::deactivate() {
    DEBUG_ASSERT(self->active);
    ON_DEBUG(self->active = false);
    doDeactivate();
}

void RenderTarget::beginScene() {
    DEBUG_ASSERT(self->active);
    if (self->viewport_changed) {
        self->viewport_changed = false;
        doViewport(self->effectiveViewport());
    }
}

void RenderTarget::clear(uint32 buffers) {
    DEBUG_ASSERT_MSG(self->active, "RenderTarget not active");
    buffers &= self->buffers;
    doClear(buffers);
}

void RenderTarget::draw() {
    DEBUG_ASSERT(self->active);
    doDraw();
}

void RenderTarget::viewport(const Viewport& vp) {
    self->viewport_changed = vp != self->viewport;
    self->viewport = vp;
}

void RenderTarget::updateSize(uint32 w, uint32 h) {
    self->width = w;
    self->height = h;
    self->viewport_changed = self->viewport == Viewport();
}

void RenderTarget::doDeactivate() {
    // noop
}

void RenderTarget::doClear(uint32 buffers) {
    GLbitfield bits = 0;

    if (buffers & RT_COLOR_BUFFER)
        bits |= GL_COLOR_BUFFER_BIT;

    if (buffers & RT_DEPTH_BUFFER)
        bits |= GL_DEPTH_BUFFER_BIT;

    if (buffers & RT_STENCIL_BUFFER)
        bits |= GL_STENCIL_BUFFER_BIT;

    if (bits != 0)
        GL_CHECK(glClear(bits));
}

void RenderTarget::doDraw() {
    // noop
}

void RenderTarget::doViewport(const Viewport& vp) {
    GL_CHECK(glViewport(vp.offsetX, vp.offsetY, vp.width, vp.height));
}

} // namespace glt
