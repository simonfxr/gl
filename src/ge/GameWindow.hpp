#ifndef GE_GAMEWINDOW_HPP
#define GE_GAMEWINDOW_HPP

#include <string>

#include <SFML/Graphics.hpp>

#include "ge/conf.hpp"
#include "ge/WindowRenderTarget.hpp"
#include "ge/Event.hpp"
#include "ge/EngineEvents.hpp"
#include "ge/Command.hpp"
#include "ge/KeyBinding.hpp"

namespace ge {

using namespace defs;

struct WindowEvents;

struct GE_API WindowOptions {
    size width;
    size height;
    std::string title;
    sf::ContextSettings settings;
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
    GameWindow(sf::RenderWindow& win);
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
    
    sf::RenderWindow& window();

    bool focused() const;

    bool init();

    WindowRenderTarget& renderTarget();

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
    sf::Event::KeyEvent key;
    bool pressed;
    KeyChanged(GameWindow& win, bool press, const sf::Event::KeyEvent& e) :
        WindowEvent(win), key(e), pressed(press) {}
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
    bool pressed;
    sf::Event::MouseButtonEvent button;
    MouseButton(GameWindow& win, bool press, index16 _x, index16 _y, const sf::Event::MouseButtonEvent& butn) :
        WindowEvent(win),
        x(_x), y(_y),
        pressed(press), button(butn) {}
};

struct GE_API FocusChanged : public WindowEvent {
    bool focused;
    FocusChanged(GameWindow& win, bool focus) : WindowEvent(win), focused(focus) {}
};

} // namespace ge

#endif
