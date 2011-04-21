#ifndef GLT_TEXTURE_HANDLE_HPP
#define GLT_TEXTURE_HANDLE_HPP

#include "opengl.h"

namespace glt {

enum TextureType {
    Texture1D,
    Texture2D,
    Texture3D
};

struct TextureHandle {
private:
    
    GLuint _handle;
    TextureType _type;
    
public:

    TextureHandle();
    TextureHandle(TextureType type);
    
    ~TextureHandle();

    void free();
    void bind();
    
    GLuint handle() const { return _handle; } 
    TextureType type() const { return _type; }
    
    void type(TextureType new_type);
    
private:
    TextureHandle(const TextureHandle& _);
    TextureHandle& operator =(const TextureHandle& _);
};

static const TextureHandle TEXTURE_NULL(Texture2D);

bool operator ==(const TextureHandle& t1, const TextureHandle& t2);
bool operator !=(const TextureHandle& t1, const TextureHandle& t2);

} // namespace glt

#endif
