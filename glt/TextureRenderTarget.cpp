#include "glt/TextureRenderTarget.hpp"
#include "opengl.h"
#include "glt/utils.hpp"

namespace glt {

TextureRenderTarget::TextureRenderTarget(uint32 w, uint32 h, uint32 bs, uint32 ss) :
    RenderTarget(0, 0, bs),
    texture(Texture2D, ss),
    frame_buffer(0),
    depth_buffer(0),
    samples(ss)
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

        if (samples == 1)
            GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h));
        else
            GL_CHECK(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, w, h));
                
        GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer));
    }

    GLenum status;
    GL_CHECK(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE) {

        const char *str_status = "unknown framebuffer status";
#define CASE(s) case s: str_status = #s; break
        switch (status) {
            CASE(GL_FRAMEBUFFER_UNSUPPORTED);
            CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
            CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
            CASE(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
            CASE(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
            CASE(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
            CASE(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
//            CASE(GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT);
        }
#undef CASE
        
        ERR(str_status);
    }
}

void TextureRenderTarget::createTexture(bool delete_old) {

    if (delete_old)
        texture.free();

    texture.type(Texture2D, samples);
    texture.bind();
    GLenum ttype = texture.glType();

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    if (ttype == GL_TEXTURE_2D) {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
    } else if (ttype == GL_TEXTURE_2D_MULTISAMPLE) {
        GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, texture.samples(), GL_RGBA, width(), height(), GL_FALSE));
    } else {
        ERR("unexpected Texture type");
    }

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));    
    GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.handle(), 0));
}

void TextureRenderTarget::doActivate() {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
}

} // namespace glt
