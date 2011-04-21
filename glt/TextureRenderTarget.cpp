#include "glt/TextureRenderTarget.hpp"
#include "opengl.h"
#include "glt/utils.hpp"

namespace glt {

TextureRenderTarget::TextureRenderTarget(uint32 w, uint32 h, uint32 bs) :
    RenderTarget(0, 0, bs),
    texture(Texture2D),
    frame_buffer(0),
    depth_buffer(0)
{
    resize(w, h);
}

TextureRenderTarget::~TextureRenderTarget() {
    GL_CHECK(glDeleteFramebuffers(1, &frame_buffer));
    GL_CHECK(glDeleteRenderbuffers(1, &depth_buffer));
}

TextureHandle& TextureRenderTarget::textureHandle() {
    return texture;
}

void TextureRenderTarget::resize(uint32 w, uint32 h) {

    if (width() == w && height() == h)
        return;

    ASSERT_MSG(buffers() & RT_COLOR_BUFFER, "TextureRenderTarget without RT_COLOR_BUFFERS makes no sense");
    ASSERT_MSG(!(buffers() & RT_STENCIL_BUFFER), "StencilBuffer not yet supported");

    updateSize(w, h);

    GL_CHECK(glDeleteFramebuffers(1, &frame_buffer));
    GL_CHECK(glDeleteRenderbuffers(1, &depth_buffer));

    frame_buffer = 0;
    depth_buffer = 0;

    GL_CHECK(glGenFramebuffers(1, &frame_buffer));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));

    createTexture();

    if (buffers() & RT_DEPTH_BUFFER) {
        GL_CHECK(glGenRenderbuffers(1, &depth_buffer));
        GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer));
        GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h));
        GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer));
    }

    GLenum status;
    GL_CHECK(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE) {

        const char *str_status = "unknown";
#define CASE(s) case s: str_status = #s; break
        switch (status) {
            CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
            CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
            CASE(GL_FRAMEBUFFER_UNSUPPORTED);
        }
#undef CASE
        
        ERROR(str_status);
    }
}

void TextureRenderTarget::createTexture(bool delete_old) {

    if (delete_old)
        texture.free();

    texture.type(Texture2D);
    texture.bind();
    
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));    
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.handle(), 0));
}

void TextureRenderTarget::doActivate() {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
}

} // namespace glt
