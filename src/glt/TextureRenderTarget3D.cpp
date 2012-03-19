#include "glt/TextureRenderTarget3D.hpp"
#include "err/err.hpp"
#include "glt/utils.hpp"
#include "math/vec3.hpp"
#include "math/ivec3.hpp"

namespace glt {

TextureRenderTarget3D::TextureRenderTarget3D(const math::ivec3_t& s, const TextureRenderTarget3D::Params& ps) :
    TextureRenderTarget(0, 0, ps),
    _depth(SIZE(s[2])),
    _color_format(ps.color_format),
    _target_attachment(Attachment(AttachmentLayer, 0))
{
    resize(s);
}

void TextureRenderTarget3D::resize(const math::ivec3_t& s) {
    size w = SIZE(s[0]), h = SIZE(s[1]), d = SIZE(s[2]);
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

void TextureRenderTarget3D::createTexture(bool delete_old) {
    if (delete_old)
        _sampler.data()->free();
    
    _sampler.data()->type(Texture3D);
    
    _sampler.data()->bind(0, false);
    GL_CHECK(glTexImage3D(GL_TEXTURE_3D, 0, GLint(_color_format), width(), height(), _depth,
                          0, GL_RGB, GL_UNSIGNED_BYTE, 0));
    _sampler.data()->unbind(0, false);

    _sampler.filterMode(this->filterMode());
    _sampler.clampMode(this->clampMode());

    Attachment current = _target_attachment;
    AttachmentType other;

    switch (current.type) {
    case AttachmentLayered: other = AttachmentLayer; break;
    case AttachmentLayer: other = AttachmentLayered; break;
    default: ASSERT_FAIL();
    }

    _target_attachment.type = other;  // force update
    _target_attachment.index = 0;
    targetAttachment(current);
}

void TextureRenderTarget3D::targetAttachment(const TextureRenderTarget3D::Attachment& ta) {
    if (ta.type != _target_attachment.type ||
        (ta.type == AttachmentLayer && ta.index != _target_attachment.index)) {
        
        _target_attachment = ta;
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer));

        switch (ta.type) {
        case AttachmentLayered:
            GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _sampler.data()->handle(), 0));
            break;
        case AttachmentLayer:
            _sampler.data()->bind(0, false);
            GL_CHECK(glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, _sampler.data()->handle(), 0, ta.index));
            // _sampler.data()->bind();
            // GL_CHECK(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _sampler.data()->handle(), ta.index, 0));    
            _sampler.data()->unbind(0, false);
            break;
        default: ASSERT_FAIL();
        }
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));        
    }
}

} // namespace glt
