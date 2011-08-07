#include <vector>
#include <limits>
#include <iostream>

#include "defs.h"
#include "math/real.hpp"
#include "error/error.hpp"
#include "glt/utils.hpp"
#include "ge/GameWindow.hpp"
#include "ge/Event.hpp"

namespace ge {


namespace {

static void resizeRenderTarget(const Event<WindowResized>& e) {
    e.info.window.renderTarget().resized();
}

} // namespace anon

struct GameWindow::Data {
    GameWindow &self;

    const bool owning_win;
    sf::RenderWindow *win;
    
    bool grab_mouse;
    bool have_focus;

    int32 mouse_x;
    int32 mouse_y;

    bool show_mouse_cursor;
    bool accum_mouse_moves;

    WindowRenderTarget *renderTarget;

    WindowEvents events;

    Data(GameWindow& _self, const WindowOptions& opts) :
        self(_self), owning_win(true),
        win(new sf::RenderWindow(sf::VideoMode(opts.width, opts.height),
                                 opts.title,
                                 sf::Style::Default,
                                 opts.settings)) {}
    
    Data(GameWindow& _self, sf::RenderWindow& w) : 
        self(_self), owning_win(false),  win(&w) {}
        
    ~Data();

    void init();
    void handleInputEvents();
    void setMouse(int x, int y);

    static void runHandleInputEvents(Data *, const Event<EngineEvent>&);
};

void GameWindow::Data::init() {
    grab_mouse = false;
    have_focus = true;
    show_mouse_cursor = true;
    accum_mouse_moves = true;

    mouse_x = 0;
    mouse_y = 0;

    renderTarget = new WindowRenderTarget(self);
    win->SetActive();

    win->EnableKeyRepeat(false);
    win->EnableVerticalSync(false);
    
    events.windowResized.reg(makeEventHandler(resizeRenderTarget));
}

void GameWindow::Data::setMouse(int x, int y) {
    sf::Mouse::SetPosition(sf::Vector2i(x, y), *win);
}

GameWindow::Data::~Data() {
    if (owning_win) delete win;
    delete renderTarget;
}

GameWindow::GameWindow(const WindowOptions& opts) :
    self(new Data(*this, opts)) { self->init(); }

GameWindow::GameWindow(sf::RenderWindow& win) :
    self(new Data(*this, win)) { self->init(); }

GameWindow::~GameWindow() {
    delete self;
}

bool GameWindow::init() {
    return true;
}

void GameWindow::Data::handleInputEvents() {

    int32 mouse_current_x = mouse_x;
    int32 mouse_current_y = mouse_y;

    bool was_resize = false;
    uint32 new_w = 0;
    uint32 new_h = 0;

    sf::Event e;
    while (win->PollEvent(e)) {
        
        switch (e.Type) {
        case sf::Event::Closed:
            events.windowClosed.raise(makeEvent(WindowEvent(self)));
            break;
            
        case sf::Event::Resized:
            was_resize = true;
            new_w = e.Size.Width;
            new_h = e.Size.Height;
            break;
            
        case sf::Event::KeyPressed:
            events.keyChanged.raise(makeEvent(KeyChanged(self, true, e.Key)));
            break;
            
        case sf::Event::KeyReleased:
            events.keyChanged.raise(makeEvent(KeyChanged(self, false, e.Key)));
            break;
            
        case sf::Event::MouseMoved:
            mouse_x = e.MouseMove.X;
            mouse_y = e.MouseMove.Y;
            
            if (!accum_mouse_moves) {
                MouseMoved ev(self);
                ev.dx = mouse_current_x - mouse_x;
                ev.dy = mouse_y - mouse_current_y;
                ev.x = mouse_current_x = mouse_x;
                ev.y = mouse_current_y = mouse_y;
                if (have_focus && (ev.dx != 0 || ev.dy != 0)) {
                    events.mouseMoved.raise(makeEvent(ev));
                }
            }
            
            break;
            
        case sf::Event::MouseButtonPressed:
            mouse_x = e.MouseButton.X;
            mouse_y = e.MouseButton.Y;
            events.mouseButton.raise(makeEvent(MouseButton(self, true, e.MouseButton)));
            break;

        case sf::Event::MouseButtonReleased:
            mouse_x = e.MouseButton.X;
            mouse_y = e.MouseButton.Y;
            events.mouseButton.raise(makeEvent(MouseButton(self, false, e.MouseButton)));
            break;
            
        case sf::Event::LostFocus:
            
            if (have_focus) {
                have_focus = false;
                win->ShowMouseCursor(true);
                events.focusChanged.raise(makeEvent(FocusChanged(self, false)));
            }
            
            break;
            
        case sf::Event::GainedFocus:

            if (!have_focus) {
                have_focus = true;

                if (!show_mouse_cursor)
                    win->ShowMouseCursor(false);
            
                if (grab_mouse) {
                    uint32 win_w = win->GetWidth();
                    uint32 win_h = win->GetHeight();
                
                    mouse_current_x = mouse_x = win_w / 2;
                    mouse_current_y = mouse_y = win_h / 2;

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
            mouse_current_x = mouse_x = new_w / 2;
            mouse_current_y = mouse_y = new_h / 2;
            setMouse(mouse_x, mouse_y);
        }

        events.windowResized.raise(makeEvent(WindowResized(self, new_w, new_h)));
    } else {

        int32 dx = mouse_current_x - mouse_x;
        int32 dy = mouse_y - mouse_current_y;

        if (have_focus && (dx != 0 || dy != 0)) {

            MouseMoved ev(self);
            ev.dx = dx; ev.dy = dy;
            ev.x = mouse_x; ev.y = mouse_y;

            if (grab_mouse) {
                mouse_x = win->GetWidth() / 2;
                mouse_y = win->GetHeight() / 2;

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
        self->win->ShowMouseCursor(show);
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
    
sf::RenderWindow& GameWindow::window() {
    return *self->win;
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

void GameWindow::Data::runHandleInputEvents(Data *win, const Event<EngineEvent>&) {
    win->handleInputEvents();
}

void GameWindow::registerHandlers(EngineEvents& evnts) {
    evnts.handleInput.reg(makeEventHandler(Data::runHandleInputEvents, self));
}

} // namespace ge
