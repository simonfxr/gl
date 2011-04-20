#include "glt/TextureRenderTarget.hpp"
#include "opengl.h"
#include "glt/utils.hpp"

namespace glt {

TextureRenderTarget::TextureRenderTarget(uint32 w, uint32 h, uint32 bs) :
    RenderTarget(0, 0, bs),
    frame_buffer(0),
    depth_buffer(0)
{
    resize(w, h);
}

TextureRenderTarget::~TextureRenderTarget() {
    if (frame_buffer != 0)
        GL_CHECK(glDeleteFramebuffers(1, &frame_buffer));

    if (depth_buffer != 0)
        GL_CHECK(glDeleteRenderbuffers(1, &depth_buffer));
}

const TextureHandle& TextureRenderTarget::textureHandle() {
    return texture;
}

void TextureRenderTarget::resize(uint32 w, uint32 h) {

    if (width() == w && height() == h)
        return;

    updateSize(w, h);

    GL_CHECK(glDeleteFramebuffers(1, &frame_buffer));
    GL_CHECK(glDeleteRenderbuffers(1, &depth_buffer));
    GL_CHECK(glDeleteTextures(1, &texture.texture));

    texture.texture = 0;
    frame_buffer = 0;
    depth_buffer = 0;
    
    GLuint tex = 0;
    GL_CHECK(glGenTextures(1, &tex));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));

    texture.texture = tex;

    GL_CHECK(glGenFramebuffers(1, &frame_buffer));
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer));

    GL_CHECK(glGenRenderbuffers(1, &depth_buffer));
    GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer));
    GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h));
    GL_CHECK(glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer));

    GL_CHECK(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0));

    GLenum status;
    GL_CHECK(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

    const char *str_status = "unknown";
#define CASE(s) case s: str_status = #s; break
    switch (status) {
        CASE(GL_FRAMEBUFFER_COMPLETE);
        CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
//        CASE(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS);
        CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
        CASE(GL_FRAMEBUFFER_UNSUPPORTED);
    }
#undef CASE

    if (status != GL_FRAMEBUFFER_COMPLETE)
        ERROR(str_status);
}

void TextureRenderTarget::doActivate() {
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer));
}

void TextureRenderTarget::doDeactivate() {
    // noop
}

void TextureRenderTarget::doDraw() {
    // noop
}

} // namespace glt
