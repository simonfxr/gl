#ifndef GLT_TEXTURE_DATA_HPP
#define GLT_TEXTURE_DATA_HPP

#include "glt/conf.hpp"
#include "opengl.hpp"

namespace glt {

using namespace defs;

enum TextureType {
    Texture1D,
    Texture2D,
    Texture3D
};

struct GLT_API TextureData {
private:
    size _samples;
    GLuint _handle;
    TextureType _type;
    
public:
    TextureData() : _samples(1), _handle(0), _type(Texture2D) {}
    TextureData(TextureType type, size samples = 1);

    ~TextureData();

    void free();

    void bind(uint32 idx, bool set_active_idx = true);
    void unbind(uint32 idx, bool set_active_idx = true);

    GLuint ensureHandle();
    GLuint handle() const { return _handle; }
    size samples() const { return _samples; }
    TextureType type() { return _type; }
    GLenum glType() const;

    void type(TextureType new_type, size new_samples = 1);

private:
    TextureData(const TextureData&);
    TextureData& operator =(const TextureData&);
};

inline bool operator ==(const TextureData& t1, const TextureData& t2) {
    return t1.handle() == t2.handle();
}

inline bool operator !=(const TextureData& t1, const TextureData& t2) {
    return !(t1 == t2);
}

} // namespace glt

#endif
