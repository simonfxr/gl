#ifndef GLT_WINDOW_RENDER_TARGET_HPP
#define GLT_WINDOW_RENDER_TARGET_HPP

#include "defs.h"
#include "glt/RenderTarget.hpp"
#include "ge/GameWindow.hpp"

namespace glt {

struct WindowRenderTarget EXPLICIT : public RenderTarget {
private:

    ge::GameWindow& window;

public:

    WindowRenderTarget(ge::GameWindow& win);
    
protected:
    void doActivate() OVERRIDE;
    void doDeactivate() OVERRIDE;
    void doDraw() OVERRIDE;
    void doViewport(const Viewport& vp) OVERRIDE;
};

} // namespace glt

#endif
