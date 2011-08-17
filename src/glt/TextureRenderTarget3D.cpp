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
    _targetAttachment(Attachment(AttachmentLayer, 0))
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
        texture.free();
    
    texture.type(Texture3D);
    texture.bind();

    GL_CHECK(glTexImage3D(GL_TEXTURE_3D, 0, GLint(_color_format), width(), height(), _depth, 0, GL_RGB, GL_UNSIGNED_BYTE, 0));

    texture.filterMode(this->defaultFilterMode());

    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT));

    Attachment current = _targetAttachment;
    AttachmentType other;

    switch (current.type) {
    case AttachmentLayered: other = AttachmentLayer; break;
    case AttachmentLayer: other = AttachmentLayered; break;
    default: ASSERT_FAIL();
    }

    _targetAttachment.type = other;  // force update
    _targetAttachment.index = 0;
    targetAttachment(current);
}

void TextureRenderTarget3D::targetAttachment(const TextureRenderTarget3D::Attachment& ta) {
    if (ta.type != _targetAttachment.type ||
        (ta.type == AttachmentLayer && ta.index != _targetAttachment.index)) {
        
        _targetAttachment = ta;
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));

        switch (ta.type) {
        case AttachmentLayered:
            GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.handle(), 0));
            break;
        case AttachmentLayer:
            texture.bind();
            GL_CHECK(glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, texture.handle(), 0, ta.index));
            // texture.bind();
            // GL_CHECK(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.handle(), ta.index, 0));    
            break;
        default: ASSERT_FAIL();
        }

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));        
    }
}

} // namespace glt
