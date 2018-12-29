#include "glt/TextureRenderTarget.hpp"
#include "err/err.hpp"
#include "glt/utils.hpp"
#include "opengl.hpp"

namespace glt {

TextureRenderTarget::TextureRenderTarget(size w,
                                         size h,
                                         const TextureRenderTarget::Params &ps)
  : RenderTarget(0, 0, ps.buffers)
  , _sampler(std::make_shared<TextureData>(Texture2D, ps.samples))
  , _frame_buffer(0)
  , _depth_buffer(0)
  , _samples(ps.samples)
  , _filter_mode(ps.filter_mode)
  , _clamp_mode(ps.clamp_mode)
{
    resize(w, h);
}

TextureRenderTarget::~TextureRenderTarget() = default;

void
TextureRenderTarget::resize(size w, size h)
{

    if (width() == w && height() == h)
        return;

    ASSERT_MSG(buffers() & RT_COLOR_BUFFER,
               "TextureRenderTarget without Colorbuffer not supported");
    ASSERT_MSG(!(buffers() & RT_STENCIL_BUFFER),
               "StencilBuffer not yet supported");

    updateSize(w, h);

    _frame_buffer.release();
    _depth_buffer.release();

    _frame_buffer.ensure();
    createTexture();

    if (buffers() & RT_DEPTH_BUFFER) {
        _depth_buffer.ensure();

        if (_samples == 1)
            GL_CALL(glNamedRenderbufferStorageEXT,
                    *_depth_buffer,
                    GL_DEPTH_COMPONENT,
                    w,
                    h);
        else
            GL_CALL(glNamedRenderbufferStorageMultisampleEXT,
                    *_depth_buffer,
                    _samples,
                    GL_DEPTH_COMPONENT,
                    w,
                    h);

        GL_CALL(glNamedFramebufferRenderbufferEXT,
                *_frame_buffer,
                GL_DEPTH_ATTACHMENT,
                GL_RENDERBUFFER,
                *_depth_buffer);
    }

    checkFramebufferStatus(_frame_buffer, GL_FRAMEBUFFER);
}

void
TextureRenderTarget::createTexture(bool delete_old)
{
    if (delete_old)
        _sampler.data()->free();

    _sampler.data()->type(Texture2D, _samples);
    GLenum ttype = _sampler.data()->glType();

    _sampler.filterMode(_filter_mode);
    _sampler.clampMode(_clamp_mode);

    _sampler.data()->bind(0, false);
    if (ttype == GL_TEXTURE_2D) {
        GL_CALL(glTexImage2D,
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                GLsizei(width()),
                GLsizei(height()),
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                nullptr);
    } else if (ttype == GL_TEXTURE_2D_MULTISAMPLE) {
        GL_CALL(glTexImage2DMultisample,
                GL_TEXTURE_2D_MULTISAMPLE,
                _samples,
                GL_RGBA,
                width(),
                height(),
                GL_FALSE);
    } else {
        ERR("unexpected Texture type");
    }
    _sampler.data()->unbind(0, false);

    GL_CALL(glNamedFramebufferTextureEXT,
            *_frame_buffer,
            GL_COLOR_ATTACHMENT0,
            *_sampler.data()->handle(),
            0);
}

void
TextureRenderTarget::doActivate()
{
    GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, *_frame_buffer);
}

void
TextureRenderTarget::doDeactivate()
{
    GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

bool
TextureRenderTarget::checkFramebufferStatus(GLFramebufferObject &buffer,
                                            GLenum target)
{
    GLenum status;
    GL_ASSIGN_CALL(status, glCheckNamedFramebufferStatusEXT, *buffer, target);
    if (status == GL_FRAMEBUFFER_COMPLETE)
        return true;

    const char *str_status = "unknown framebuffer status";
#define CASE(s)                                                                \
    case s:                                                                    \
        str_status = #s;                                                       \
        break
    switch (status) {
        CASE(GL_FRAMEBUFFER_UNDEFINED);
        CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
        CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
        CASE(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
        CASE(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
        CASE(GL_FRAMEBUFFER_UNSUPPORTED);
        CASE(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
        CASE(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
    }
#undef CASE

    ERR(str_status);
    return false;
}

} // namespace glt
