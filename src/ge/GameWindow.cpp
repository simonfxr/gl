#include <vector>
#include <limits>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

#include "defs.hpp"
#include "math/real.hpp"
#include "err/err.hpp"
#include "glt/utils.hpp"
#include "ge/GameWindow.hpp"
#include "ge/Event.hpp"

namespace ge {


namespace {

KeyCode fromSFML(sf::Keyboard::Key key);

KeyCode fromSFML(sf::Mouse::Button button);

KeyCode fromSFML(sf::Keyboard::Key key) {
    return static_cast<KeyCode>(key);
}

KeyCode fromSFML(sf::Mouse::Button button) {
    return KeyCode(int32(button) + int32(keycode::KeyCount) + 1);
}

static void resizeRenderTarget(const Event<WindowResized>& e) {
    e.info.window.renderTarget().resized();
}

template <typename T, typename U>
static void copyContextInfo(const T& a, U& b) {
    b.depthBits = a.depthBits;
    b.stencilBits = a.stencilBits;
    b.antialiasingLevel = a.antialiasingLevel;
    b.majorVersion = a.majorVersion;
    b.minorVersion = a.minorVersion;
    b.debugContext = a.debugContext;
    b.coreProfile = a.coreProfile;
}

static sf::ContextSettings toSFContextSettings(const GLContextInfo& a) {
    sf::ContextSettings b;
    copyContextInfo(a, b);
    return b;
}

} // namespace anon

struct GameWindow::Data {
    GameWindow &self;

    const bool owning_win;
    sf::RenderWindow *win;
    
    bool grab_mouse;
    bool have_focus;
    bool vsync;

    index16 mouse_x;
    index16 mouse_y;

    bool show_mouse_cursor;
    bool accum_mouse_moves;

    WindowRenderTarget *renderTarget;

    WindowEvents events;

    Data(GameWindow& _self, bool owns_win, sf::RenderWindow *rw) : 
        self(_self),
        owning_win(owns_win),
        win(rw),
        grab_mouse(false),
        have_focus(true),
        vsync(false),
        mouse_x(0),
        mouse_y(0),
        show_mouse_cursor(true),
        accum_mouse_moves(true),
        renderTarget(0),
        events()        
        {}
        
    ~Data();

    void init(const WindowOptions& opts);
    void handleInputEvents();
    void setMouse(int x, int y);

    static void runHandleInputEvents(Data *, const Event<InputEvent>&);
};

void GameWindow::Data::init(const WindowOptions& opts) {

    
    ASSERT(int(keycode::KeyCount) == int(sf::Keyboard::KeyCount));
    ASSERT(int(keycode::Count) - int(keycode::KeyCount) - 1 == int(sf::Mouse::ButtonCount));

    
    ASSERT(renderTarget == 0);
    renderTarget = new WindowRenderTarget(self);
    win->setActive();

    win->setKeyRepeatEnabled(false);
    
    vsync = opts.vsync;
    win->setVerticalSyncEnabled(opts.vsync);
    
    events.windowResized.reg(makeEventHandler(resizeRenderTarget));
}

void GameWindow::Data::setMouse(int x, int y) {
    sf::Mouse::setPosition(sf::Vector2i(x, y), *win);
}

GameWindow::Data::~Data() {
    delete renderTarget;
    
    if (owning_win) {
        delete win;
    }
}

GameWindow::GameWindow(const WindowOptions& opts) :
    self(new Data(*this,
                  true,
                  new sf::RenderWindow(sf::VideoMode(uint32(opts.width), uint32(opts.height)),
                                       opts.title,
                                       sf::Style::Default,
                                       toSFContextSettings(opts.settings)))) { self->init(opts); }

GameWindow::~GameWindow() {
    delete self;
}

bool GameWindow::init() {
    return true;
}

void GameWindow::Data::handleInputEvents() {

    index16 mouse_current_x = mouse_x;
    index16 mouse_current_y = mouse_y;

    bool was_resize = false;
    size new_w = 0;
    size new_h = 0;

    sf::Event e;
    while (win->pollEvent(e)) {
        
        switch (e.type) {
        case sf::Event::Closed:
            events.windowClosed.raise(makeEvent(WindowEvent(self)));
            break;
            
        case sf::Event::Resized:
            was_resize = true;
            new_w = SIZE(e.size.width);
            new_h = SIZE(e.size.height);
            break;
            
        case sf::Event::KeyPressed: {
            Key key = Key::make(keystate::Pressed, fromSFML(e.key.code));
            events.keyChanged.raise(makeEvent(KeyChanged(self, key)));
            break;
        }
        case sf::Event::KeyReleased: {
            Key key = Key::make(keystate::Released, fromSFML(e.key.code));
            events.keyChanged.raise(makeEvent(KeyChanged(self, key)));
            break;
        }
        case sf::Event::MouseMoved:
            mouse_x = index16(e.mouseMove.x);
            mouse_y = index16(e.mouseMove.y);
            
            if (!accum_mouse_moves) {
                int16 dx = int16(mouse_current_x - mouse_x);
                int16 dy = int16(mouse_y - mouse_current_y);
                
                mouse_current_x = mouse_x;
                mouse_current_y = mouse_y;

                if (have_focus && (dx != 0 || dy != 0)) {
                    MouseMoved ev(self, dx, dy, mouse_x, mouse_y);
                    events.mouseMoved.raise(makeEvent(ev));
                }
            }
            
            break;
            
        case sf::Event::MouseButtonPressed: {
            mouse_x = index16(e.mouseButton.x);
            mouse_y = index16(e.mouseButton.y);
            Key button = Key::make(keystate::Pressed, fromSFML(e.mouseButton.button));
            events.mouseButton.raise(makeEvent(MouseButton(self, mouse_x, mouse_y, button)));
            break;
        }
        case sf::Event::MouseButtonReleased: {
            mouse_x = index16(e.mouseButton.x);
            mouse_y = index16(e.mouseButton.y);
            Key button = Key::make(keystate::Released, fromSFML(e.mouseButton.button));
            events.mouseButton.raise(makeEvent(MouseButton(self, mouse_x, mouse_y, button)));
            break;
        }
        case sf::Event::LostFocus:
            
            if (have_focus) {
                have_focus = false;
                win->setMouseCursorVisible(true);
                events.focusChanged.raise(makeEvent(FocusChanged(self, false)));
            }
            
            break;
            
        case sf::Event::GainedFocus:

            if (!have_focus) {
                have_focus = true;

                if (!show_mouse_cursor)
                    win->setMouseCursorVisible(false);
            
                if (grab_mouse) {
                    uint32 win_w = win->getSize().x;
                    uint32 win_h = win->getSize().y;
                
                    mouse_current_x = mouse_x = index16(win_w / 2);
                    mouse_current_y = mouse_y = index16(win_h / 2);

                    setMouse(mouse_x, mouse_y);
                }

                events.focusChanged.raise(makeEvent(FocusChanged(self, true)));
            }
            
            break;

        case sf::Event::TextEntered: break;
        case sf::Event::MouseWheelMoved: break;
        case sf::Event::MouseEntered: break;
        case sf::Event::MouseLeft: break;
        case sf::Event::JoystickButtonPressed: break;
        case sf::Event::JoystickButtonReleased: break;
        case sf::Event::JoystickMoved: break;
        case sf::Event::JoystickConnected: break;
        case sf::Event::JoystickDisconnected: break;
        case sf::Event::Count: FATAL_ERR("not possible?"); break;
        default:
            ERR("unknown SFML-Eventtype");
            break;
        }
    }

    if (was_resize) {

        if (grab_mouse) {
            mouse_current_x = mouse_x = index16(new_w / 2);
            mouse_current_y = mouse_y = index16(new_h / 2);
            setMouse(mouse_x, mouse_y);
        }

        events.windowResized.raise(makeEvent(WindowResized(self, new_w, new_h)));
    } else {

        int16 dx = int16(mouse_current_x - mouse_x);
        int16 dy = int16(mouse_y - mouse_current_y);
        mouse_current_x = mouse_x;
        mouse_current_y = mouse_y;

        if (have_focus && (dx != 0 || dy != 0)) {

            MouseMoved ev(self, dx, dy, mouse_x, mouse_y);

            if (grab_mouse) {
                mouse_x = index16(win->getSize().x / 2);
                mouse_y = index16(win->getSize().y / 2);

                setMouse(mouse_x, mouse_y);
            }

            events.mouseMoved.raise(makeEvent(ev));
        }
    }
}

void GameWindow::grabMouse(bool grab) {
    self->grab_mouse = grab;
}

bool GameWindow::grabMouse() const {
    return self->grab_mouse;
}

void GameWindow::showMouseCursor(bool show) {
    if (show != self->show_mouse_cursor) {
        self->show_mouse_cursor = show;
        self->win->setMouseCursorVisible(show);
    }
}

bool GameWindow::showMouseCursor() const {
    return self->show_mouse_cursor;
}

void GameWindow::accumulateMouseMoves(bool accum) {
    self->accum_mouse_moves = accum;
}

bool GameWindow::accumulateMouseMoves() {
    return self->accum_mouse_moves;
}

void GameWindow::vsync(bool enable) {
    if (self->vsync != enable) {
        self->vsync = enable;
        self->win->setVerticalSyncEnabled(enable);
    }
}

bool GameWindow::vsync() {
    return self->vsync;
}

bool GameWindow::focused() const {
    return self->have_focus;
}

WindowRenderTarget& GameWindow::renderTarget(){
    return *self->renderTarget;
}

WindowEvents& GameWindow::events() {
    return self->events;
}

void GameWindow::Data::runHandleInputEvents(Data *win, const Event<InputEvent>&) {
    win->handleInputEvents();
}

void GameWindow::registerHandlers(EngineEvents& evnts) {
    evnts.handleInput.reg(makeEventHandler(Data::runHandleInputEvents, self));
}

size GameWindow::windowHeight() const {
    return self->win->getSize().y;
}

size GameWindow::windowWidth() const {
    return self->win->getSize().x;
}

void GameWindow::setActive() {
    self->win->setActive();
}

void GameWindow::swapBuffers() {
    self->win->display();
}

void GameWindow::setMouse(index16 x, index16 y) {
    self->setMouse(x, y);
}

void GameWindow::contextInfo(GLContextInfo& info) const {
    copyContextInfo(self->win->getSettings(), info);
}


} // namespace ge
