#ifndef GLT_TEXTURE_HANDLE_HPP
#define GLT_TEXTURE_HANDLE_HPP

#include "opengl.h"

namespace glt {

struct TextureHandle {
    GLuint texture;

    TextureHandle();
    TextureHandle(GLuint texture_id);
    
    ~TextureHandle();

private:
    TextureHandle(const TextureHandle& _);
    TextureHandle& operator =(const TextureHandle& _);
};

bool operator ==(const TextureHandle& t1, const TextureHandle& t2);
bool operator !=(const TextureHandle& t1, const TextureHandle& t2);

} // namespace glt

#endif
