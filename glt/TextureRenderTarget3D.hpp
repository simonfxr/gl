#ifndef GLT_TEXTURE_RENDER_TARGET_3D_HPP
#define GLT_TEXTURE_RENDER_TARGET_3D_HPP

#include "defs.h"
#include "glt/TextureRenderTarget.hpp"

namespace glt {

struct TextureRenderTarget3D EXPLICIT : public TextureRenderTarget {
protected:

    uint32 _depth;
    uint32 _targetDepth;
        
public:

    TextureRenderTarget3D(uint32 w, uint32 h, uint32 d, uint32 bs);
    void resize(uint32 w, uint32 h, uint32 d);

    uint32 depth() { return _depth; }
    uint32 targetDepth() { return _targetDepth; }
    void targetDepth(uint32 td);
    
    virtual void createTexture(bool delete_old = true) EXPLICIT;
};

} // namespace glt

#endif

