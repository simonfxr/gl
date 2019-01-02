#ifndef GE_GAMEWINDOW_HPP
#define GE_GAMEWINDOW_HPP

#include "ge/conf.hpp"

#include "ge/Command.hpp"
#include "ge/EngineEvents.hpp"
#include "ge/Event.hpp"
#include "ge/KeyBinding.hpp"
#include "ge/WindowRenderTarget.hpp"

#include <memory>
#include <string>

namespace ge {

struct WindowEvents;

struct GE_API GLContextInfo
{
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

struct GE_API WindowOptions
{
    defs::size_t width;
    defs::size_t height;
    std::string title;
    GLContextInfo settings;
    bool vsync;

    WindowOptions()
      : width(800), height(600), title(""), settings(), vsync(false)
    {}
};

struct GE_API GameWindow
{
    GameWindow(const WindowOptions &opts = WindowOptions());

    void showMouseCursor(bool show);
    bool showMouseCursor() const;

    void vsync(bool enable);
    bool vsync();

    bool focused() const;

    bool init();

    WindowRenderTarget &renderTarget();

    void swapBuffers();

    void contextInfo(GLContextInfo &info) const;

    defs::size_t windowHeight() const;

    defs::size_t windowWidth() const;

    void windowSize(defs::size_t &width, defs::size_t &height) const;

    void setMouse(defs::int16_t, defs::int16_t);

    WindowEvents &events();

    void registerHandlers(EngineEvents &evnts);

private:
    DECLARE_PIMPL(self);
};

struct WindowEvent;
struct WindowResized;
struct FramebufferResized;
struct KeyChanged;
struct MouseMoved;
struct MouseButton;
struct FocusChanged;

struct GE_API WindowEvents
{
    EventSource<WindowEvent> windowClosed;
    EventSource<FocusChanged> focusChanged;
    EventSource<WindowResized> windowResized;
    EventSource<FramebufferResized> framebufferResized;
    EventSource<KeyChanged> keyChanged;
    EventSource<MouseMoved> mouseMoved;
    EventSource<MouseButton> mouseButton;
};

struct GE_API WindowEvent
{
    GameWindow &window;
    explicit WindowEvent(GameWindow &win) : window(win) {}
};

struct GE_API WindowResized : public WindowEvent
{
    defs::size_t width, height;
    WindowResized(GameWindow &win, defs::size_t w, defs::size_t h)
      : WindowEvent(win), width(w), height(h)
    {}
};

struct GE_API FramebufferResized : public WindowEvent
{
    defs::size_t width, height;
    FramebufferResized(GameWindow &win, defs::size_t w, defs::size_t h)
      : WindowEvent(win), width(w), height(h)
    {}
};

struct GE_API KeyChanged : public WindowEvent
{
    Key key;
    KeyChanged(GameWindow &win, const Key &k) : WindowEvent(win), key(k) {}
};

struct GE_API MouseMoved : public WindowEvent
{
    defs::int16_t dx, dy;
    defs::int16_t x, y;
    MouseMoved(GameWindow &win,
               defs::int16_t _dx,
               defs::int16_t _dy,
               defs::int16_t _x,
               defs::int16_t _y)
      : WindowEvent(win), dx(_dx), dy(_dy), x(_x), y(_y)
    {}
};

struct GE_API MouseButton : public WindowEvent
{
    defs::int16_t x, y;
    Key button;

    MouseButton(GameWindow &win,
                defs::int16_t _x,
                defs::int16_t _y,
                const Key &butn)
      : WindowEvent(win), x(_x), y(_y), button(butn)
    {}
};

struct GE_API FocusChanged : public WindowEvent
{
    bool focused;
    FocusChanged(GameWindow &win, bool focus) : WindowEvent(win), focused(focus)
    {}
};

} // namespace ge

#endif
