#include "glt/RenderTarget.hpp"
#include "glt/utils.hpp"

#include <SFML/OpenGL.hpp>

namespace glt {

struct RenderTarget::Data {
    uint32 width;
    uint32 height;
    uint32 buffers;
    Viewport viewport;

    bool active;
    bool viewport_changed;

    Data(uint32 w, uint32 h, uint32 bs, const Viewport& vp) :
        width(w), height(h), buffers(bs),
        viewport(vp == Viewport() ? Viewport(w, h) : vp),
        active(false),
        viewport_changed(false) {}
};

RenderTarget::RenderTarget(uint32 w, uint32 h, uint32 bs, const Viewport& vp) :
    self(new Data(w, h, bs, vp))
{}

RenderTarget::~RenderTarget() {
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
    if (!self->active) {
        self->active = true;
        doActivate();

        if (self->viewport_changed) {
            self->viewport_changed = false;
            doViewport(self->viewport);
        }
    }
}

void RenderTarget::deactivate() {
    if (self->active) {
        self->active = false;
        doDeactivate();
    }
}

void RenderTarget::clear(uint32 buffers, glt::color clear_color) {
    ASSERT_MSG(self->active, "RenderTarget not active");
    buffers &= self->buffers;
    doClear(buffers, clear_color);
}

void RenderTarget::draw() {
    ASSERT_MSG(self->active, "RenderTarget not active");
    doDraw();
}

void RenderTarget::viewport(const Viewport& vp) {
    if (vp != self->viewport) {
        self->viewport = vp;

        if (self->active)
            doViewport(vp);
        self->viewport_changed = !self->active;
    }
}

void RenderTarget::updateSize(uint32 w, uint32 h) {
    self->width = w;
    self->height = h;
}

void RenderTarget::doDeactivate() {
    // noop
}

void RenderTarget::doClear(uint32 buffers, glt::color clear_color) {
    GLbitfield bits = 0;

    if (buffers & RT_COLOR_BUFFER) {
        bits |= GL_COLOR_BUFFER_BIT;
        const math::vec4_t c = clear_color.vec4();
        GL_CHECK(glClearColor(c.r, c.g, c.b, c.a));
    }

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
