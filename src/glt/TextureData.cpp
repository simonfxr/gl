#include "glt/TextureData.hpp"
#include "err/err.hpp"
#include "glt/utils.hpp"

namespace glt {

namespace {

GLenum
getGLType(TextureType ty, size_t ss)
{
    switch (ty) {
    case Texture1D:
        return GL_TEXTURE_1D;
    case Texture2D:
        if (ss == 1)
            return GL_TEXTURE_2D;
        else
            return GL_TEXTURE_2D_MULTISAMPLE;
    case Texture3D:
        return GL_TEXTURE_3D;
    }
    UNREACHABLE;
}

} // namespace

TextureData::TextureData(TextureType ty, size_t samples)
  : _samples(samples), _handle(0), _type(ty)
{}

TextureData::~TextureData()
{
    free();
}

void
TextureData::free()
{
    _handle.release();
}

GLTextureObject &
TextureData::ensureHandle()
{
    return _handle.ensure();
}

void
TextureData::bind(uint32_t idx, bool set_active_idx)
{
    if (set_active_idx)
        GL_CALL(glActiveTexture, GL_TEXTURE0 + idx);
    ensureHandle();
    GL_CALL(glBindTexture, getGLType(_type, _samples), *_handle);
}

void
TextureData::unbind(uint32_t idx, bool set_active_idx)
{
    if (set_active_idx)
        GL_CALL(glActiveTexture, GL_TEXTURE0 + idx);
    GL_CALL(glBindTexture, getGLType(_type, _samples), 0);
}

GLenum
TextureData::glType() const
{
    return getGLType(_type, _samples);
}

void
TextureData::type(TextureType ty, size_t ss)
{
    ASSERT((ty == _type && ss == _samples) || *_handle == 0,
           "cannot change type, texture already created");
    _type = ty;
    _samples = ss;
}

void
TextureData::upload(const Image &img)
{

    GLenum format{};
    GLenum type{};
    switch (img.format()) {
    case ImageFormat::RGB:
        format = GL_RGB8;
        type = GL_RGBA;
        break;
    case ImageFormat::RGBA:
        format = GL_RGBA8;
        type = GL_RGBA;
    }

    auto old_dim = bl::exchange(_allocated_dim, img.dim());
    auto old_format = bl::exchange(_allocated_format, img.format());

    if (_handle && (old_dim[0] * old_dim[1]) != 0 &&
        old_format == _allocated_format && old_dim == _allocated_dim) {
        GL_CALL(glTexSubImage2DEXT,
                *ensureHandle(),
                GL_TEXTURE_2D,
                0,
                0,
                GLint(img.width()),
                GLint(img.heigth()),
                format,
                type,
                img.data());
    } else {
        _handle.release();
        GL_CALL(glTextureImage2DEXT,
                *ensureHandle(),
                GL_TEXTURE_2D,
                0,
                format,
                GLint(img.width()),
                GLint(img.heigth()),
                0,
                type,
                GL_UNSIGNED_BYTE,
                img.data());
    }
}

} // namespace glt
