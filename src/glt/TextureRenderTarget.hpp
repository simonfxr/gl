#ifndef GLT_TEXTURE_RENDER_TARGET_HPP
#define GLT_TEXTURE_RENDER_TARGET_HPP

#include "glt/conf.hpp"
#include "opengl.hpp"
#include "glt/RenderTarget.hpp"
#include "glt/TextureSampler.hpp"

namespace glt {

using namespace defs;

struct GLT_API TextureRenderTarget EXPLICIT : public RenderTarget {
    TextureSampler _sampler;
    GLuint _frame_buffer;
    GLuint _depth_buffer;
    size _samples;
    GLenum _color_format;
    TextureSampler::FilterMode _filter_mode;
    TextureSampler::ClampMode _clamp_mode;
    
    TextureSampler& sampler() { return _sampler; }

    struct Params {
        RenderTargetBuffers buffers;
        size samples;
        TextureSampler::FilterMode filter_mode;
        TextureSampler::ClampMode clamp_mode;

        Params() :
            buffers(RT_COLOR_BUFFER),
            samples(1),
            filter_mode(TextureSampler::FilterNearest),
            clamp_mode(TextureSampler::ClampToEdge)
            {}
    };

    TextureRenderTarget(size w, size h, const Params&);
    ~TextureRenderTarget();

    void resize(size width, size height);

    TextureSampler::FilterMode filterMode() { return _filter_mode; }
    void filterMode(TextureSampler::FilterMode m) { _filter_mode = m; }

    TextureSampler::ClampMode clampMode() { return _clamp_mode; }
    void clampMode(TextureSampler::ClampMode m) { _clamp_mode = m; }
    
    virtual void createTexture(bool delete_old = true);
    
    static bool checkFramebufferStatus(GLuint buffer, GLenum target);

protected:
    virtual void doDeactivate() OVERRIDE;
    virtual void doActivate() OVERRIDE;
};

} // namespace glt

#endif
