#ifndef GLT_WINDOW_RENDER_TARGET_HPP
#define GLT_WINDOW_RENDER_TARGET_HPP

#include "defs.h"
#include "glt/RenderTarget.hpp"

namespace ge {

struct GameWindow;

struct WindowRenderTarget EXPLICIT : public glt::RenderTarget {
private:

    GameWindow& window;

public:

    WindowRenderTarget(GameWindow& win);
    
protected:
    void doActivate() OVERRIDE;
    void doDeactivate() OVERRIDE;
    void doDraw() OVERRIDE;
    void doViewport(const glt::Viewport& vp) OVERRIDE;
};

} // namespace ge

#endif
