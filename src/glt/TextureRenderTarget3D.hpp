#ifndef GLT_TEXTURE_RENDER_TARGET_3D_HPP
#define GLT_TEXTURE_RENDER_TARGET_3D_HPP

#include "glt/TextureRenderTarget.hpp"
#include "glt/conf.hpp"
#include "math/ivec3.hpp"
#include "opengl.hpp"

namespace glt {

struct GLT_API TextureRenderTarget3D final : public TextureRenderTarget
{
public:
    enum AttachmentType
    {
        AttachmentLayered,
        AttachmentLayer
    };

    struct Attachment
    {
        AttachmentType type;
        size_t size{};
    };

protected:
    size_t _depth;
    GLenum _color_format;
    Attachment _target_attachment;

public:
    struct Params
    {
        TextureRenderTarget::Params texture;
        GLenum color_format = GL_RGB8;
    };

    TextureRenderTarget3D(const math::ivec3_t &, const Params &);
    void resize(const math::ivec3_t &);

    size_t depth() const { return _depth; }
    Attachment targetAttachment() const { return _target_attachment; }
    void targetAttachment(const Attachment &);

    virtual void createTexture(bool delete_old) override;

    std::shared_ptr<TextureRenderTarget3D> make_shared(
      const math::ivec3_t &dims,
      const Params &ps)
    {
        auto p = std::make_shared<TextureRenderTarget3D>(dims, ps);
        p->resize(dims);
        return p;
    }
};

} // namespace glt

#endif
