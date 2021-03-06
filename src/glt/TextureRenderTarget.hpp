#ifndef GLT_TEXTURE_RENDER_TARGET_HPP
#define GLT_TEXTURE_RENDER_TARGET_HPP

#include "glt/GLObject.hpp"
#include "glt/RenderTarget.hpp"
#include "glt/TextureSampler.hpp"
#include "glt/conf.hpp"
#include "opengl.hpp"

namespace glt {

struct GLT_API TextureRenderTarget : public RenderTarget
{
    TextureSampler _sampler;
    GLFramebufferObject _frame_buffer;
    GLRenderbufferObject _depth_buffer;
    size_t _samples;
    TextureSampler::FilterMode _filter_mode;
    TextureSampler::ClampMode _clamp_mode;

    TextureSampler &sampler() { return _sampler; }

    struct Params
    {
        RenderTargetBuffers buffers;
        size_t samples;
        TextureSampler::FilterMode filter_mode;
        TextureSampler::ClampMode clamp_mode;

        Params()
          : buffers(RT_COLOR_BUFFER)
          , samples(1)
          , filter_mode(TextureSampler::FilterNearest)
          , clamp_mode(TextureSampler::ClampToEdge)
        {}
    };

    TextureRenderTarget(size_t w, size_t h, const Params &);
    virtual ~TextureRenderTarget() override;

    void resize(size_t width, size_t height);

    TextureSampler::FilterMode filterMode() { return _filter_mode; }
    void filterMode(TextureSampler::FilterMode m) { _filter_mode = m; }

    TextureSampler::ClampMode clampMode() { return _clamp_mode; }
    void clampMode(TextureSampler::ClampMode m) { _clamp_mode = m; }

    virtual void createTexture(bool delete_old = true);

    static bool checkFramebufferStatus(GLFramebufferObject &fb, GLenum target);

protected:
    virtual void doDeactivate() override;
    virtual void doActivate() override;
};

} // namespace glt

#endif
