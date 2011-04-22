#include "defs.h"
#include "glt/TextureHandle.hpp"
#include "glt/utils.hpp"

namespace glt {

namespace {

GLenum glType(TextureType ty) {
    switch (ty) {
    case Texture1D: return GL_TEXTURE_1D;
    case Texture2D: return GL_TEXTURE_2D;
    case Texture3D: return GL_TEXTURE_3D;
    default:
        ERR("invalid TextureType");
        return GL_FALSE;
    }
}

} // namespace anon

TextureHandle::TextureHandle(TextureType ty) :
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
    GL_CHECK(glBindTexture(glType(_type), _handle));
}

void TextureHandle::type(TextureType ty) {
    ASSERT_MSG(ty == _type || _handle == 0, "cannot change type, texture already created");
    _type = ty;
}

bool operator ==(const TextureHandle& t1, const TextureHandle& t2) {
    return t1.handle() == t2.handle();
}

bool operator !=(const TextureHandle& t1, const TextureHandle& t2) {
    return !(t1 == t2);
}

} // namespace glt
