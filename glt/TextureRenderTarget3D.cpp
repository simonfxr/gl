#include "glt/TextureRenderTarget3D.hpp"
#include "glt/utils.hpp"

namespace glt {

TextureRenderTarget3D::TextureRenderTarget3D(uint32 w, uint32 h, uint32 d, uint32 bs) :
    TextureRenderTarget(0, 0, bs),
    _depth(d),
    _targetDepth(0)
{
    resize(w, h, d);
}

void TextureRenderTarget3D::resize(uint32 w, uint32 h, uint32 d) {
    
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

    GL_CHECK(glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, width(), height(), _depth, 0, GL_RGB, GL_UNSIGNED_BYTE, 0));

    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

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
