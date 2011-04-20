#ifndef GLT_TEXTURE_RENDER_TARGET_HPP
#define GLT_TEXTURE_RENDER_TARGET_HPP

#include "defs.h"
#include "glt/RenderTarget.hpp"
#include "glt/TextureHandle.hpp"

namespace glt {

struct TextureRenderTarget EXPLICIT : public RenderTarget {
    const TextureHandle& textureHandle();

    TextureRenderTarget(uint32 w, uint32 h, uint32 buffers);
    ~TextureRenderTarget();

    void resize(uint32 width, uint32 height);

protected:
    void doActivate() OVERRIDE;
    void doDeactivate() OVERRIDE;
    void doDraw() OVERRIDE;

private:
    TextureHandle texture;
    GLuint frame_buffer;
    GLuint depth_buffer;
};

} // namespace glt

#endif
