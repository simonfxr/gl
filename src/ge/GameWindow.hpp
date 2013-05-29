#ifndef GE_GAMEWINDOW_HPP
#define GE_GAMEWINDOW_HPP

#include <string>

#include "ge/conf.hpp"
#include "ge/WindowRenderTarget.hpp"
#include "ge/Event.hpp"
#include "ge/EngineEvents.hpp"
#include "ge/Command.hpp"
#include "ge/KeyBinding.hpp"

namespace ge {

using namespace defs;

struct WindowEvents;

struct GE_API GLContextInfo {
    unsigned int depthBits;
    unsigned int stencilBits;
    unsigned int antialiasingLevel;
    unsigned int majorVersion;
    unsigned int minorVersion;
    bool debugContext;
    bool coreProfile;

    GLContextInfo()
        : depthBits(8)
        , stencilBits(0)
        , antialiasingLevel(0)
        , majorVersion(0)
        , minorVersion(0)
        , debugContext(false)
        , coreProfile(false)
        {}
};

struct GE_API WindowOptions {
    size width;
    size height;
    std::string title;
    GLContextInfo settings;
    bool vsync;
    
    WindowOptions() :
        width(800),
        height(600),
        title(""),
        settings(),
        vsync(false)
        {}
};

struct GE_API GameWindow {
    GameWindow(const WindowOptions& opts = WindowOptions());
    ~GameWindow();

    void grabMouse(bool grab);
    bool grabMouse() const;

    void showMouseCursor(bool show);
    bool showMouseCursor() const;

    void accumulateMouseMoves(bool accum);
    bool accumulateMouseMoves();

    void vsync(bool enable);
    bool vsync();
    
    bool focused() const;

    bool init();

    WindowRenderTarget& renderTarget();

    void setActive();

    void swapBuffers();

    void contextInfo(GLContextInfo& info) const;

    size windowHeight() const;

    size windowWidth() const;

    WindowEvents& events();

    void registerBinding(const KeyBinding& bind, Ref<Command>& com);

    bool unregisterBinding(const KeyBinding& bind);

    void registerHandlers(EngineEvents& evnts);

private:
    
    GameWindow(const GameWindow& _);
    GameWindow& operator =(const GameWindow& _);

    struct Data;
    Data * const self;
};

struct WindowEvent;
struct WindowResized;
struct KeyChanged;
struct MouseMoved;
struct MouseButton;
struct FocusChanged;

struct GE_API WindowEvents {
    EventSource<WindowEvent> windowClosed;
    EventSource<FocusChanged> focusChanged;
    EventSource<WindowResized> windowResized;
    EventSource<KeyChanged> keyChanged;
    EventSource<MouseMoved> mouseMoved;
    EventSource<MouseButton> mouseButton;
};

struct GE_API WindowEvent {
    virtual ~WindowEvent() {}
    GameWindow& window;
    explicit WindowEvent(GameWindow& win) : window(win) {}
};

struct GE_API WindowResized : public WindowEvent {
    size width, height;
    WindowResized(GameWindow& win, size w, size h) :
        WindowEvent(win), width(w), height(h) {}
};

struct GE_API KeyChanged : public WindowEvent {
    Key key;
    KeyChanged(GameWindow& win, const Key& k) :
        WindowEvent(win), key(k) {}
};

struct GE_API MouseMoved : public WindowEvent {
    int16 dx, dy;
    index16 x, y;
    MouseMoved(GameWindow& win, int16 _dx, int16 _dy, index16 _x, index16 _y) :
        WindowEvent(win),
        dx(_dx), dy(_dy), x(_x), y(_y) {}
};

struct GE_API MouseButton : public WindowEvent {
    index16 x, y;
    Key button;
    
    MouseButton(GameWindow& win, index16 _x, index16 _y, const Key& butn) :
        WindowEvent(win),
        x(_x), y(_y),
        button(butn) {}
};

struct GE_API FocusChanged : public WindowEvent {
    bool focused;
    FocusChanged(GameWindow& win, bool focus) : WindowEvent(win), focused(focus) {}
};

} // namespace ge

#endif
