#include "glt/TextureRenderTarget3D.hpp"
#include "err/err.hpp"
#include "glt/utils.hpp"
#include "math/fvec.hpp"
#include "math/ivec.hpp"

namespace glt {

TextureRenderTarget3D::TextureRenderTarget3D(
  const math::ivec3 &s,
  const TextureRenderTarget3D::Params &ps)
  : TextureRenderTarget(ps.texture)
  , _depth(s[2])
  , _color_format(ps.color_format)
  , _target_attachment(Attachment{ AttachmentLayer, 0 })
{
    resize(s);
}

void
TextureRenderTarget3D::resize(const math::ivec3 &s)
{
    auto [w, h, d] = s.cast<size_t>();

    if (width() == w && height() == h) {
        if (depth() == d)
            return;
        _depth = d;
        createTexture(true);
    } else {
        _depth = d;
        TextureRenderTarget::resize(w, h);
    }
}

void
TextureRenderTarget3D::createTexture(bool delete_old)
{
    if (delete_old)
        _sampler.data()->free();

    _sampler.data()->type(Texture3D);
    auto tex = *_sampler.data()->ensureHandle();

    GL_CALL(glTextureStorage3D,
            tex,
            1,
            GLint(_color_format),
            GLsizei(width()),
            GLsizei(height()),
            GLsizei(_depth));

    _sampler.filterMode(this->filterMode());
    _sampler.clampMode(this->clampMode());

    auto current = _target_attachment;
    AttachmentType other{};

    switch (current.type) {
    case AttachmentLayered:
        other = AttachmentLayer;
        break;
    case AttachmentLayer:
        other = AttachmentLayered;
        break;
    }

    _target_attachment.type = other; // force update
    _target_attachment.size = 0;
    targetAttachment(current);
}

void
TextureRenderTarget3D::targetAttachment(
  const TextureRenderTarget3D::Attachment &ta)
{
    if (ta.type != _target_attachment.type ||
        (ta.type == AttachmentLayer && ta.size != _target_attachment.size)) {

        _target_attachment = ta;
        GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, *_frame_buffer);

        switch (ta.type) {
        case AttachmentLayered:
            GL_CALL(glFramebufferTexture,
                    GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    *_sampler.data()->handle(),
                    0);
            break;
        case AttachmentLayer:
            _sampler.data()->bind(0, false);
            GL_CALL(glFramebufferTexture3D,
                    GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_3D,
                    *_sampler.data()->handle(),
                    0,
                    GLsizei(ta.size));
            // _sampler.data()->bind();
            // GL_CALL(glFramebufferTextureLayer, GL_FRAMEBUFFER,
            // GL_COLOR_ATTACHMENT0, _sampler.data()->handle(), ta.size_t, 0);
            _sampler.data()->unbind(0, false);
            break;
        }
        GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    }
}

} // namespace glt
