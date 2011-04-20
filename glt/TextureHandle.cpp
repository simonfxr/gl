#include "glt/TextureHandle.hpp"
#include "glt/utils.hpp"

namespace glt {

TextureHandle::TextureHandle() : texture(0) {}

TextureHandle::TextureHandle(GLuint id) : texture(id) {}

TextureHandle::~TextureHandle() {
    GL_CHECK(glDeleteTextures(1, &texture));
}

bool operator ==(const TextureHandle& t1, const TextureHandle& t2) {
    return t1.texture == t2.texture;
}

bool operator !=(const TextureHandle& t1, const TextureHandle& t2) {
    return !(t1 == t2);
}

} // namespace glt
