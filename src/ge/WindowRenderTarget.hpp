#ifndef GLT_WINDOW_RENDER_TARGET_HPP
#define GLT_WINDOW_RENDER_TARGET_HPP

#include "ge/conf.hpp"
#include "glt/RenderTarget.hpp"

namespace ge {

struct GameWindow;

struct GE_API WindowRenderTarget EXPLICIT : public glt::RenderTarget {
private:
    GameWindow& window;

public:
    WindowRenderTarget(GameWindow& win);
    void resized();
    
protected:
    void doActivate() OVERRIDE;
    void doDraw() OVERRIDE;
};

} // namespace ge

#endif
