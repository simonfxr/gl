#include "glt/TextureRenderTarget3D.hpp"
#include "err/err.hpp"
#include "glt/utils.hpp"
#include "math/ivec3.hpp"

namespace glt {

TextureRenderTarget3D::TextureRenderTarget3D(const math::ivec3_t& s, const TextureRenderTarget3D::Params& ps) :
    TextureRenderTarget(0, 0, ps),
    _depth(s[2]),
    _targetDepth(0),
    _color_format(ps.color_format)
{
    resize(s);
}

void TextureRenderTarget3D::resize(const math::ivec3_t& s) {
    uint32 w = s[0], h = s[1], d = s[2];
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

    GL_CHECK(glTexImage3D(GL_TEXTURE_3D, 0, _color_format, width(), height(), _depth, 0, GL_RGB, GL_UNSIGNED_BYTE, 0));

    texture.filterMode(this->defaultFilterMode());

    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT));

    uint32 layer = _targetDepth;
    _targetDepth = layer + 1; // force update
    targetDepth(layer);
}

void TextureRenderTarget3D::targetDepth(uint32 td) {
    if (td != _targetDepth) {
        _targetDepth = td;
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));    
        GL_CHECK(glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, texture.handle(), 0, td));
    }
}

} // namespace glt
