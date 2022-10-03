#include "glt/TextureRenderTarget3D.hpp"
#include "err/err.hpp"
#include "glt/utils.hpp"
#include "math/ivec3.hpp"
#include "math/vec3.hpp"

namespace glt {

TextureRenderTarget3D::TextureRenderTarget3D(
  const math::ivec3_t &s,
  const TextureRenderTarget3D::Params &ps)
  : TextureRenderTarget(ps.texture)
  , _depth(s[2])
  , _color_format(ps.color_format)
  , _target_attachment(Attachment{ AttachmentLayer, 0 })
{
    resize(s);
}

void
TextureRenderTarget3D::resize(const math::ivec3_t &s)
{
    size_t w = s[0];

    size_t h = s[1];

    size_t d = s[2];
    if (width() == w && height() == h) {

        if (depth() == d)
            return;

        // just bind a new texture
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

    _sampler.data()->bind(0, false);
    GL_CALL(glTexImage3D,
            GL_TEXTURE_3D,
            0,
            GLint(_color_format),
            GLsizei(width()),
            GLsizei(height()),
            GLsizei(_depth),
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            nullptr);
    _sampler.data()->unbind(0, false);

    _sampler.filterMode(this->filterMode());
    _sampler.clampMode(this->clampMode());

    Attachment current = _target_attachment;
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
