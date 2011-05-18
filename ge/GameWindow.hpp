#ifndef GE_GAMEWINDOW_HPP
#define GE_GAMEWINDOW_HPP

#include <string>

#include <SFML/Graphics.hpp>

#include "defs.h"
#include "ge/WindowRenderTarget.hpp"
#include "ge/Event.hpp"
#include "ge/EngineEvents.hpp"

namespace ge {

struct WindowEvent {
    GameWindow& window;
    WindowEvent(GameWindow& win) : window(win) {}
};

struct WindowResized : public WindowEvent {
    uint32 width, height;
    WindowResized(GameWindow& win, uint32 w, uint32 h) :
        WindowEvent(win), width(w), height(h) {}
};

struct KeyChanged : public WindowEvent {
    sf::Event::KeyEvent key;
    bool pressed;
    KeyChanged(GameWindow& win, bool press, const sf::Event::KeyEvent& e) :
        WindowEvent(win), key(e), pressed(press) {}
};

struct MouseMoved : public WindowEvent {
    int32 dx, dy;
    uint32 x, y;
    MouseMoved(GameWindow& win) : WindowEvent(win) {}
};

struct MouseButton : public WindowEvent {
    uint32 x, y;
    bool pressed;
    sf::Event::MouseButtonEvent button;
    MouseButton(GameWindow& win, bool press, const sf::Event::MouseButtonEvent& butn) :
        WindowEvent(win), pressed(press), button(butn) {}
};

struct FocusChanged : public WindowEvent {
    bool focused;
    FocusChanged(GameWindow& win, bool focus) : WindowEvent(win), focused(focus) {}
};

struct WindowEvents {
    EventSource<WindowEvent> windowClosed;
    EventSource<FocusChanged> focusChanged;
    EventSource<WindowResized> windowResized;
    EventSource<KeyChanged> keyChanged;
    EventSource<MouseMoved> mouseMoved;
    EventSource<MouseButton> mouseButton;
};

struct WindowOpts {
    uint32 width;
    uint32 height;
    std::string title;
    sf::ContextSettings settings;
    
    WindowOpts() :
        width(640),
        height(480),
        title(""),
        settings(24, 0, 0, 3, 3)
        {}
};

enum KeyState {
    Up,
    Pressed,
    Down,
    Released,
};

struct GameWindow {
    GameWindow(sf::RenderWindow& win);
    GameWindow(const WindowOpts& opts = WindowOpts());
    ~GameWindow();

    KeyState keyState(sf::Key::Code key) const;
    
    KeyState mouseButtonState(sf::Mouse::Button button) const;

    void grabMouse(bool grab = true);
    bool grabMouse() const;

    void showMouseCursor(bool show = true);
    bool showMouseCursor() const;

    void accumulateMouseMoves(bool accum = true);
    bool accumulateMouseMoves();
    
    sf::RenderWindow& window();

    bool focused() const;

    bool init();

    WindowRenderTarget& renderTarget();

    WindowEvents& events();

    void registerHandlers(EngineEvents& evnts);

private:
    
    GameWindow(const GameWindow& _);
    GameWindow& operator =(const GameWindow& _);

    struct Data;
    Data * const self;
};

} // namespace ge

#endif
