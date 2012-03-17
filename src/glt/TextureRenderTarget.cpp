#include "glt/TextureRenderTarget.hpp"
#include "opengl.hpp"
#include "err/err.hpp"
#include "glt/utils.hpp"
#include "glt/glstate.hpp"


namespace glt {

TextureRenderTarget::TextureRenderTarget(size w, size h, const TextureRenderTarget::Params& ps) :
    RenderTarget(0, 0, ps.buffers),
    texture(Texture2D, ps.samples),
    frame_buffer(0),
    depth_buffer(0),
    samples(ps.samples),
    default_filter_mode(ps.default_filter_mode)
{
    resize(w, h);
}

TextureRenderTarget::~TextureRenderTarget() {
    if (frame_buffer != 0)
        --glstate.num_framebuffers;
    if (depth_buffer != 0)
        --glstate.num_renderbuffers;

    GL_CHECK(glDeleteFramebuffers(1, &frame_buffer));
    GL_CHECK(glDeleteRenderbuffers(1, &depth_buffer));
}

TextureHandle& TextureRenderTarget::textureHandle() {
    DEBUG_ASSERT(this != 0);
    return texture;
}

void TextureRenderTarget::resize(size w, size h) {

    if (width() == w && height() == h)
        return;

    ASSERT_MSG(buffers() & RT_COLOR_BUFFER, "TextureRenderTarget without RT_COLOR_BUFFERS makes no sense");
    ASSERT_MSG(!(buffers() & RT_STENCIL_BUFFER), "StencilBuffer not yet supported");

    updateSize(w, h);

    if (frame_buffer != 0)
        --glstate.num_framebuffers;
    if (depth_buffer != 0)
        --glstate.num_renderbuffers;

    GL_CHECK(glDeleteFramebuffers(1, &frame_buffer));
    GL_CHECK(glDeleteRenderbuffers(1, &depth_buffer));

    frame_buffer = 0;
    depth_buffer = 0;
    
    ++glstate.num_framebuffers;

    GL_CHECK(glGenFramebuffers(1, &frame_buffer));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));

    createTexture();

    if (buffers() & RT_DEPTH_BUFFER) {
        ++glstate.num_renderbuffers;
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

    texture.filterMode(default_filter_mode);

    if (ttype == GL_TEXTURE_2D) {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GLsizei(width()), GLsizei(height()), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
    } else if (ttype == GL_TEXTURE_2D_MULTISAMPLE) {
        GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, texture.samples(), GL_RGBA, width(), height(), GL_FALSE));
    } else {
        ERR("unexpected Texture type");
    }

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
    GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.handle(), 0));
}

void TextureRenderTarget::doDeactivate() {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}


void TextureRenderTarget::doActivate() {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
}

} // namespace glt
