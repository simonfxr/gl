#include "defs.h"
#include "glt/TextureHandle.hpp"
#include "error/error.hpp"
#include "glt/utils.hpp"


namespace glt {

namespace {

GLenum getGLType(TextureType ty, uint32 ss) {

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

TextureHandle::TextureHandle(TextureType ty, uint32 samples) :
    _samples(samples),
    _handle(0),
    _type(ty)
{}

TextureHandle::~TextureHandle() {
    free();
}

void TextureHandle::free() {
    GL_CHECK(glDeleteTextures(1, &_handle));
    _handle = 0;
}

void TextureHandle::bind() {
    if (_handle == 0)
        GL_CHECK(glGenTextures(1, &_handle));
    GL_CHECK(glBindTexture(getGLType(_type, _samples), _handle));
}

void TextureHandle::bind(uint32 active_index) {
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + active_index));
    bind();
}

GLenum TextureHandle::glType() const {
    return getGLType(_type, _samples);
}

void TextureHandle::type(TextureType ty, uint32 ss) {
    ASSERT_MSG((ty == _type && ss == _samples)  || _handle == 0, "cannot change type, texture already created");
    _type = ty;
    _samples = ss;
}

bool operator ==(const TextureHandle& t1, const TextureHandle& t2) {
    return t1.handle() == t2.handle();
}

bool operator !=(const TextureHandle& t1, const TextureHandle& t2) {
    return !(t1 == t2);
}

} // namespace glt
