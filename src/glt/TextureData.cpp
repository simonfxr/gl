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
    return _handle.ensure(getGLType(_type, _samples));
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
    GLenum glformat{};
    switch (img.format()) {
    case ImageFormat::RGB:
        glformat = GL_RGB8;
        format = GL_RGBA;
        break;
    case ImageFormat::RGBA:
        glformat = GL_RGBA8;
        format = GL_RGBA;
    }

    auto old_dim = bl::exchange(_allocated_dim, img.dim());
    auto old_format = bl::exchange(_allocated_format, img.format());

    auto storageUnchanged =
      _handle && old_format == _allocated_format && old_dim == _allocated_dim;

    using namespace bl::literals;
    if (!storageUnchanged) {
        _handle.release();
        ensureHandle();
        GL_CALL(glTextureStorage2D,
                *ensureHandle(),
                1 /* levels */,
                glformat,
                GLint(img.width()),
                GLint(img.heigth()));
    }

    GL_CALL(glTextureSubImage2D,
            *ensureHandle(),
            0 /* level */,
            0,
            0,
            GLint(img.width()),
            GLint(img.heigth()),
            format,
            GL_UNSIGNED_BYTE,
            img.data());
}

} // namespace glt
