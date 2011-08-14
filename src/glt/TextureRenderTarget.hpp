#ifndef GLT_TEXTURE_RENDER_TARGET_HPP
#define GLT_TEXTURE_RENDER_TARGET_HPP

#include "defs.h"
#include "opengl.h"
#include "glt/RenderTarget.hpp"
#include "glt/TextureHandle.hpp"

namespace glt {

struct TextureRenderTarget EXPLICIT : public RenderTarget {
    TextureHandle texture;
    GLuint frame_buffer;
    GLuint depth_buffer;
    uint32 samples;
    GLenum color_format;
    TextureHandle::FilterMode default_filter_mode;
    
    TextureHandle& textureHandle();

    struct Params {
        RenderTargetBuffers buffers;
        uint32 samples;
        TextureHandle::FilterMode default_filter_mode;

        explicit Params(RenderTargetBuffers _buffers = RT_COLOR_BUFFER,
               uint32 _samples = 1,
               TextureHandle::FilterMode _default_filter_mode = TextureHandle::FilterNearest) :
            buffers(_buffers), samples(_samples), default_filter_mode(_default_filter_mode)
            {}
    };

    TextureRenderTarget(uint32 w, uint32 h, const Params&);
    ~TextureRenderTarget();

    void resize(uint32 width, uint32 height);

    TextureHandle::FilterMode defaultFilterMode() { return default_filter_mode; }

    void defaultFilterMode(TextureHandle::FilterMode m) { default_filter_mode = m; }
    
    virtual void createTexture(bool delete_old = true);

protected:
    virtual void doActivate() OVERRIDE;
};

} // namespace glt

#endif
