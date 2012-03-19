#include "glt/TextureData.hpp"
#include "err/err.hpp"
#include "glt/utils.hpp"
#include "glt/glstate.hpp"

namespace glt {

namespace {

GLenum getGLType(TextureType ty, size ss) {
    switch (ty) {
    case Texture1D: return GL_TEXTURE_1D;
    case Texture2D:
        
        if (ss == 1)
            return GL_TEXTURE_2D;
        else
            return GL_TEXTURE_2D_MULTISAMPLE;
        
    case Texture3D: return GL_TEXTURE_3D;
    default:
        ERR("invalid TextureType");
        return GL_FALSE;
    }
}

} // namespace anon

TextureData::TextureData(TextureType ty, size samples) :
    _samples(samples),
    _handle(0),
    _type(ty)
{}

TextureData::~TextureData() {
    free();
}

void TextureData::free() {
    if (_handle != 0) {
        --glstate.num_textures;
        GL_CHECK(glDeleteTextures(1, &_handle));
    }
    _handle = 0;
}

GLuint TextureData::ensureHandle() {
    if (_handle == 0) {
        GL_CHECK(glGenTextures(1, &_handle));
        ++glstate.num_textures;
    }
    return _handle;
}

void TextureData::bind(uint32 idx, bool set_active_idx) {
    if (set_active_idx)
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + idx));
    ensureHandle();
    GL_CHECK(glBindTexture(getGLType(_type, _samples), _handle));
}

void TextureData::unbind(uint32 idx, bool set_active_idx) {
    if (set_active_idx)
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + idx));
    GL_CHECK(glBindTexture(getGLType(_type, _samples), 0));
}

GLenum TextureData::glType() const {
    return getGLType(_type, _samples);
}

void TextureData::type(TextureType ty, size ss) {
    ASSERT_MSG((ty == _type && ss == _samples) || _handle == 0,
               "cannot change type, texture already created");
    _type = ty;
    _samples = ss;
}

} // namespace glt
