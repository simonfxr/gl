#include "defs.hpp"
#include "glt/TextureHandle.hpp"
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

TextureHandle::TextureHandle(TextureType ty, size samples) :
    _samples(samples),
    _handle(0),
    _type(ty)
{}

TextureHandle::~TextureHandle() {
    free();
}

void TextureHandle::free() {
    if (_handle != 0) {
        --glstate.num_textures;
        GL_CHECK(glDeleteTextures(1, &_handle));
    }
    _handle = 0;
}

void TextureHandle::bind() {
    if (_handle == 0) {
        GL_CHECK(glGenTextures(1, &_handle));
        ++glstate.num_textures;
    }
    GL_CHECK(glBindTexture(getGLType(_type, _samples), _handle));
}

void TextureHandle::bind(uint32 active_index) {
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + active_index));
    bind();
}

GLenum TextureHandle::glType() const {
    return getGLType(_type, _samples);
}

void TextureHandle::type(TextureType ty, size ss) {
    ASSERT_MSG((ty == _type && ss == _samples)  || _handle == 0, "cannot change type, texture already created");
    _type = ty;
    _samples = ss;
}

void TextureHandle::filterMode(TextureHandle::FilterMode mode, TextureHandle::Filter filter) {
    bind();

    GLenum gltarget = getGLType(_type, _samples);
    
    GLint glmode;
    switch (mode) {
    case FilterNearest: glmode = GL_NEAREST; break;
    case FilterLinear: glmode = GL_LINEAR; break;
    default: ASSERT_FAIL_MSG("invalid FilterMode");
    }

    bool min = false;
    bool mag = false;
    switch (filter) {
    case FilterMin: min = true; break;
    case FilterMag: mag = true; break;
    case FilterMinMag: min = mag = true; break;
    default: ASSERT_FAIL_MSG("invalid Filter");
    }

    if (min)
        GL_CHECK(glTexParameteri(gltarget, GL_TEXTURE_MIN_FILTER, glmode));
    if (mag)
        GL_CHECK(glTexParameteri(gltarget, GL_TEXTURE_MAG_FILTER, glmode));
}

bool operator ==(const TextureHandle& t1, const TextureHandle& t2) {
    return t1.handle() == t2.handle();
}

bool operator !=(const TextureHandle& t1, const TextureHandle& t2) {
    return !(t1 == t2);
}

} // namespace glt
