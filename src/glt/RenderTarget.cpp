#include "glt/RenderTarget.hpp"
#include "err/err.hpp"
#include "glt/utils.hpp"
#include "opengl.hpp"

#include "math/glvec.hpp"

namespace glt {

struct RenderTarget::Data {
    size width;
    size height;
    RenderTargetBuffers buffers;
    Viewport viewport;
    color clearColor;

    bool viewport_changed;

    DEBUG_DECL(bool active;)

    Data(size w, size h, RenderTargetBuffers bs, const Viewport& vp) :
        width(w), height(h), buffers(bs),
        viewport(vp),
        clearColor(),
        viewport_changed(false)
        {
            ON_DEBUG(active = false);
        }

    Viewport effectiveViewport() const {
        return viewport == Viewport() ? Viewport(width, height) : viewport;
    }
};

RenderTarget::RenderTarget(size w, size h, RenderTargetBuffers bs, const Viewport& vp) :
    self(new Data(w, h, bs, vp))
{}

RenderTarget::~RenderTarget() {
    DEBUG_ASSERT(!self->active);
    delete self;
}

size RenderTarget::width() const {
    return self->width;
}

size RenderTarget::height() const {
    return self->height;
}

RenderTargetBuffers RenderTarget::buffers() const {
    return self->buffers;
}

color RenderTarget::clearColor() const {
    return self->clearColor;
}

void RenderTarget::clearColor(glt::color col) {
    self->clearColor = col;
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
    doClear(buffers, self->clearColor);
}

void RenderTarget::draw() {
    DEBUG_ASSERT(self->active);
    doDraw();
}

void RenderTarget::viewport(const Viewport& vp) {
    self->viewport_changed = vp != self->viewport;
    self->viewport = vp;
}

void RenderTarget::updateSize(size w, size h) {
    self->width = w;
    self->height = h;
    self->viewport_changed = self->viewport == Viewport();
}

void RenderTarget::doDeactivate() {
    // noop
}

void RenderTarget::doClear(uint32 buffers, color c) {
    GLbitfield bits = 0;

    if (buffers & RT_COLOR_BUFFER) {
        const math::vec4_t::gl col4 = c.vec4();
        GL_CALL(glClearBufferfv, GL_COLOR, 0, col4.buffer);
    }

    if (buffers & RT_DEPTH_BUFFER)
        bits |= GL_DEPTH_BUFFER_BIT;

    if (buffers & RT_STENCIL_BUFFER)
        bits |= GL_STENCIL_BUFFER_BIT;

    if (bits != 0)
        GL_CALL(glClear, bits);
}

void RenderTarget::doDraw() {
    // noop
}

void RenderTarget::doViewport(const Viewport& vp) {
    GL_CALL(glViewport, vp.offsetX, vp.offsetY, GLsizei(vp.width), GLsizei(vp.height));
}

} // namespace glt
