#ifndef GLT_TEXTURE_RENDER_TARGET_HPP
#define GLT_TEXTURE_RENDER_TARGET_HPP

#include "defs.hpp"
#include "opengl.hpp"
#include "glt/RenderTarget.hpp"
#include "glt/TextureHandle.hpp"

namespace glt {

using namespace defs;

struct TextureRenderTarget EXPLICIT : public RenderTarget {
    TextureHandle texture;
    GLuint frame_buffer;
    GLuint depth_buffer;
    size samples;
    GLenum color_format;
    TextureHandle::FilterMode default_filter_mode;
    
    TextureHandle& textureHandle();

    struct Params {
        RenderTargetBuffers buffers;
        size samples;
        TextureHandle::FilterMode default_filter_mode;

        explicit Params(RenderTargetBuffers _buffers = RT_COLOR_BUFFER,
               size _samples = 1,
               TextureHandle::FilterMode _default_filter_mode = TextureHandle::FilterNearest) :
            buffers(_buffers), samples(_samples), default_filter_mode(_default_filter_mode)
            {}
    };

    TextureRenderTarget(size w, size h, const Params&);
    ~TextureRenderTarget();

    void resize(size width, size height);

    TextureHandle::FilterMode defaultFilterMode() { return default_filter_mode; }

    void defaultFilterMode(TextureHandle::FilterMode m) { default_filter_mode = m; }
    
    virtual void createTexture(bool delete_old = true);

protected:
    virtual void doActivate() OVERRIDE;
};

} // namespace glt

#endif
