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
      , majorVersion(3)
      , minorVersion(3)
      , debugContext(true)
      , coreProfile(false)
    {}
};

struct GE_API WindowOptions
{
    size_t width;
    size_t height;
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
    ~GameWindow();

    void showMouseCursor(bool show);
    bool showMouseCursor() const;

    void vsync(bool enable);
    bool vsync();

    bool focused() const;

    static bool init();

    const std::shared_ptr<WindowRenderTarget> &renderTarget() const;

    void swapBuffers();

    GLContextInfo contextInfo() const;

    size_t windowHeight() const;

    size_t windowWidth() const;

    void windowSize(size_t &width, size_t &height) const;

    void setMouse(int16_t, int16_t);

    WindowEvents &events();

    void registerHandlers(EngineEvents &evnts);

private:
    DECLARE_PIMPL(GE_API, self);
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
    size_t width, height;
    WindowResized(GameWindow &win, size_t w, size_t h)
      : WindowEvent(win), width(w), height(h)
    {}
};

struct GE_API FramebufferResized : public WindowEvent
{
    size_t width, height;
    FramebufferResized(GameWindow &win, size_t w, size_t h)
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
    int16_t dx, dy;
    int16_t x, y;
    MouseMoved(GameWindow &win,
               int16_t _dx,
               int16_t _dy,
               int16_t _x,
               int16_t _y)
      : WindowEvent(win), dx(_dx), dy(_dy), x(_x), y(_y)
    {}
};

struct GE_API MouseButton : public WindowEvent
{
    int16_t x, y;
    Key button;

    MouseButton(GameWindow &win, int16_t _x, int16_t _y, const Key &butn)
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
