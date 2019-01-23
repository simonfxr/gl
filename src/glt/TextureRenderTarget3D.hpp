#ifndef GLT_TEXTURE_RENDER_TARGET_3D_HPP
#define GLT_TEXTURE_RENDER_TARGET_3D_HPP

#include "glt/TextureRenderTarget.hpp"
#include "math/ivec3.hpp"

namespace glt {

struct GLT_API TextureRenderTarget3D : public TextureRenderTarget
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
        size_t size;
        Attachment(AttachmentType _type, size_t _index = 0)
          : type(_type), size(_index)
        {}
    };

protected:
    size_t _depth;
    GLenum _color_format;
    Attachment _target_attachment;

public:
    struct GLT_API Params : TextureRenderTarget::Params
    {
        GLenum color_format; // default is GL_RGB8
        Params();
    };

    TextureRenderTarget3D(const math::ivec3_t &, const Params &);
    void resize(const math::ivec3_t &);

    size_t depth() const { return _depth; }
    Attachment targetAttachment() const { return _target_attachment; }
    void targetAttachment(const Attachment &);

    virtual void createTexture(bool delete_old = true) override;
};

} // namespace glt

#endif
