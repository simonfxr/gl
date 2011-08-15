#ifndef GLT_TEXTURE_HANDLE_HPP
#define GLT_TEXTURE_HANDLE_HPP

#include "opengl.hpp"

namespace glt {

using namespace defs;

enum TextureType {
    Texture1D,
    Texture2D,
    Texture3D
};

struct TextureHandle {
private:

    size _samples;
    GLuint _handle;
    TextureType _type;
    
public:

    enum FilterMode {
        FilterNearest,
        FilterLinear
    };

    enum Filter {
        FilterMin,
        FilterMag,
        FilterMinMag
    };

    TextureHandle() : _samples(1), _handle(0), _type(Texture2D)  {}
    TextureHandle(TextureType type, size samples = 1);
    
    ~TextureHandle();

    void free();
    
    void bind();
    void bind(uint32 active_index);
    
    GLuint handle() const { return _handle; }
    size samples() const { return _samples; }
    TextureType type() const { return _type; }
    GLenum glType() const;
    
    void type(TextureType new_type, size new_samples = 1);

    void filterMode(FilterMode mode, Filter filter = FilterMinMag);
    
private:
    TextureHandle(const TextureHandle& _);
    TextureHandle& operator =(const TextureHandle& _);
};

static const TextureHandle TEXTURE_NULL(Texture2D);

bool operator ==(const TextureHandle& t1, const TextureHandle& t2);
bool operator !=(const TextureHandle& t1, const TextureHandle& t2);

} // namespace glt

#endif
