#ifndef GLT_TEXTURE_DATA_HPP
#define GLT_TEXTURE_DATA_HPP

#include "glt/GLObject.hpp"
#include "glt/conf.hpp"
#include "opengl.hpp"

namespace glt {

enum TextureType
{
    Texture1D,
    Texture2D,
    Texture3D
};

struct GLT_API TextureData
{
public:
private:
    defs::size_t _samples;
    GLTextureObject _handle;
    TextureType _type;

public:
    TextureData() : _samples(1), _handle(0), _type(Texture2D) {}
    TextureData(TextureType type, defs::size_t samples = 1);

    ~TextureData();

    void free();

    void bind(defs::uint32_t idx, bool set_active_idx = true);
    void unbind(defs::uint32_t idx, bool set_active_idx = true);

    GLTextureObject &ensureHandle();
    const GLTextureObject &handle() const { return _handle; }
    size_t samples() const { return _samples; }
    TextureType type() { return _type; }
    GLenum glType() const;

    void type(TextureType ty, defs::size_t ss = 1);

    TextureData(const TextureData &) = delete;
    TextureData &operator=(const TextureData &) = delete;
};

inline bool
operator==(const TextureData &t1, const TextureData &t2)
{
    return t1.handle()._name == t2.handle()._name;
}

inline bool
operator!=(const TextureData &t1, const TextureData &t2)
{
    return !(t1 == t2);
}

} // namespace glt

#endif
