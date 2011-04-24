#ifndef GLT_TEXTURE_RENDER_TARGET_HPP
#define GLT_TEXTURE_RENDER_TARGET_HPP

#include "defs.h"
#include "glt/RenderTarget.hpp"
#include "glt/TextureHandle.hpp"

namespace glt {

struct TextureRenderTarget EXPLICIT : public RenderTarget {
    TextureHandle& textureHandle();

    TextureRenderTarget(uint32 w, uint32 h, uint32 buffers, uint32 samples = 1);
    ~TextureRenderTarget();

    void resize(uint32 width, uint32 height);
    
    virtual void createTexture(bool delete_old = true);

protected:
    virtual void doActivate() OVERRIDE;

    TextureHandle texture;
    GLuint frame_buffer;
    GLuint depth_buffer;
    uint32 samples;
};

} // namespace glt

#endif
