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
    
}

void TextureRenderTarget::resize(uint32 w, uint32 h) {

    if (width() == w && height() == h)
        return;

    updateSize(w, h);

    if (depth_buffer != 0)
        GL_CHECK(glDeleteRenderbuffers(1, &depth_buffer));

    if (frame_buffer != 0)
        GL_CHECK(glDeleteFramebuffers(1, &frame_buffer));
    
    GLuint tex;
    GL_CHECK(glGenTextures(1, &tex));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

//    byte *texels = new byte[w * h * 4];
    byte *texels = 0;

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texels));    

    // delete[] texels;
    // texels = 0;

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
#define CASE(s) case s: str_status = #s
    switch (status) {
        CASE(GL_FRAMEBUFFER_COMPLETE);
//        CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
//        CASE(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS);
        CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
        CASE(GL_FRAMEBUFFER_UNSUPPORTED);
    }
#undef CASE

    ERROR(str_status);
}

void TextureRenderTarget::doActivate() {
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer));
}

void TextureRenderTarget::doDeactivate() {
    
}

void TextureRenderTarget::doDraw() {
    
}

} // namespace glt
