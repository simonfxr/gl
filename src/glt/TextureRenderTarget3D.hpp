#ifndef GLT_TEXTURE_RENDER_TARGET_3D_HPP
#define GLT_TEXTURE_RENDER_TARGET_3D_HPP

#include "defs.hpp"
#include "opengl.hpp"
#include "glt/TextureRenderTarget.hpp"
#include "math/ivec3/type.hpp"

namespace glt {

struct TextureRenderTarget3D EXPLICIT : public TextureRenderTarget {
public:

    enum AttachmentType {
        AttachmentLayered,
        AttachmentLayer
    };

    struct Attachment {
        AttachmentType type;
        defs::index index;
        Attachment(AttachmentType _type, defs::index _index = 0) :
            type(_type), index(_index) {}
    };
    
protected:

    size _depth;
    GLenum _color_format;
    Attachment _targetAttachment;

public:

    
    struct Params : public TextureRenderTarget::Params {
        GLenum color_format;
        explicit Params(GLenum col_format = GL_RGB8, const TextureRenderTarget::Params& ps = TextureRenderTarget::Params()) :
            TextureRenderTarget::Params(ps),
            color_format(col_format)
            {}
    };

    TextureRenderTarget3D(const math::ivec3_t&, const Params&);
    void resize(const math::ivec3_t&);

    size depth() const { return _depth; }
    Attachment targetAttachment() const { return _targetAttachment; }
    void targetAttachment(const Attachment&);
    
    virtual void createTexture(bool delete_old = true) EXPLICIT;
};

} // namespace glt

#endif

