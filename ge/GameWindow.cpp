#include <vector>
#include <limits>
#include <iostream>

#include "defs.h"
#include "math/math.hpp"
#include "error/error.hpp"
#include "glt/utils.hpp"
#include "ge/GameWindow.hpp"

#ifdef SYSTEM_UNIX
#define HAVE_UNISTD
#include <unistd.h>
#endif

namespace ge {

struct KeyState {
    float keyAge;

    KeyState() : keyAge(math::POS_INF) {}
    
    KeyState(float age) : keyAge(age) {}

    bool operator ==(const KeyState& ks) const {
        return keyAge == ks.keyAge;
    }

    bool operator !=(const KeyState& ks) const {
        return !(*this == ks);
    }
};

namespace {

const KeyState KEY_STATE_UP;

} // namespace anon

struct GameWindow::Data {
    GameWindow &self;

    const bool owning_win;
    sf::RenderWindow *win;
    
    bool grab_mouse;
    bool have_focus;
    
    KeyState keyStates[sf::Key::Count];
    bool mouseStates[sf::Mouse::ButtonCount];
    sf::Clock clock;
    
    int32 mouse_x;
    int32 mouse_y;

    bool show_mouse_cursor;
    bool accum_mouse_moves;

    WindowRenderTarget *renderTarget;

    WindowEvents events;

    Data(GameWindow& _self, const WindowOpts& opts) :
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
    float now();

    struct HandleInput;
};

float GameWindow::Data::now() {
    return clock.GetElapsedTime();
}

void GameWindow::Data::init() {
    grab_mouse = false;
    have_focus = true;
    show_mouse_cursor = true;

    for (uint32 i = 0; i < ARRAY_LENGTH(keyStates); ++i)
        keyStates[i] = KEY_STATE_UP;

    for (uint32 i = 0; i < ARRAY_LENGTH(mouseStates); ++i)
        mouseStates[i] = false;

    mouse_x = 0;
    mouse_y = 0;

    renderTarget = new WindowRenderTarget(self);
    win->SetActive();

    if (grab_mouse)

    win->EnableKeyRepeat(false);
    win->EnableVerticalSync(false);

    // TODO: register handler
//    renderTarget->resized();
}

GameWindow::Data::~Data() {
    if (owning_win) delete win;
    delete renderTarget;
}

GameWindow::GameWindow(const WindowOpts& opts) :
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
            keyStates[int32(e.Key.Code)] = KeyState(now());
            events.keyChanged.raise(makeEvent(KeyChanged(self, true, e.Key)));
            break;
            
        case sf::Event::KeyReleased:
            keyStates[int32(e.Key.Code)] = KEY_STATE_UP;
            events.keyChanged.raise(makeEvent(KeyChanged(self, false, e.Key)));
            break;
            
        case sf::Event::MouseMoved:
            mouse_x = e.MouseMove.X;
            mouse_y = e.MouseMove.Y;
            
            if (!accum_mouse_moves) {
                MouseMoved ev(self);
                ev.x = mouse_current_x = mouse_x;
                ev.y = mouse_current_y = mouse_y;
                ev.dx = mouse_current_x - mouse_x;
                ev.dy = mouse_y - mouse_current_y;
                if (have_focus && (ev.dx != 0 || ev.dy != 0))
                    events.mouseMoved.raise(makeEvent(ev));
            }
            
            break;
            
        case sf::Event::MouseButtonPressed:
            mouse_x = e.MouseButton.X;
            mouse_y = e.MouseButton.Y;
            mouseStates[int32(e.MouseButton.Button)] = true;
            events.mouseButton.raise(makeEvent(MouseButton(self, true, e.MouseButton)));
            break;

        case sf::Event::MouseButtonReleased:
            mouse_x = e.MouseButton.X;
            mouse_y = e.MouseButton.Y;
            mouseStates[int32(e.MouseButton.Button)] = false;
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
            
                if (grab_mouse) {
                    win->ShowMouseCursor(false);

                    uint32 win_w = win->GetWidth();
                    uint32 win_h = win->GetHeight();
                
                    mouse_current_x = mouse_x = win_w / 2;
                    mouse_current_y = mouse_y = win_h / 2;

                    win->SetCursorPosition(mouse_x, mouse_y);
                }

                events.focusChanged.raise(makeEvent(FocusChanged(self, true)));
            }
            
            break;
        }
    }

    if (was_resize) {

        if (grab_mouse) {
            mouse_current_x = mouse_x = new_w / 2;
            mouse_current_y = mouse_y = new_h / 2;
            win->SetCursorPosition(mouse_x, mouse_y);
        }

        events.windowResized.raise(makeEvent(WindowResized(self, new_w, new_h)));
    } else {

        int32 dx = mouse_current_x - mouse_x;
        int32 dy = mouse_y - mouse_current_y;

        if (have_focus && (dx != 0 || dy != 0)) {
            if (grab_mouse) {
                mouse_x = win->GetWidth() / 2;
                mouse_y = win->GetHeight() / 2;
                win->SetCursorPosition(mouse_x, mouse_y);
            }

            MouseMoved ev(self);
            ev.dx = dx; ev.dy = dy;
            ev.x = mouse_x; ev.y = mouse_y;
            events.mouseMoved.raise(makeEvent(ev));
        }
    }
}

// bool GameWindow::init(const std::string& windowTitle, sf::RenderWindow *win, sf::Clock *clock) {

//     if (self != 0) {
//         ERR("GameWindow already initialized");
//         return false;
//     }

//     self = new Data(*this);

//     self->owning_clock = clock == 0;
//     self->clock = clock == 0 ? new sf::Clock : clock;

//     float startupT0 = self->clock->GetElapsedTime();

//     self->initialized = false;
//     self->grab_mouse = false;
//     self->game_frame_id = 0;
//     self->render_frame_id = 0;
//     self->change_pause_state = false;
//     self->new_pause_state = false;
//     self->have_focus = true;

//     for (uint32 i = 0; i < ARRAY_LENGTH(self->keyStates); ++i)
//         self->keyStates[i] = KEY_STATE_UP;

//     for (uint32 i = 0; i < ARRAY_LENGTH(self->mouseStates); ++i)
//         self->mouseStates[i] = false;

//     self->mouse_x = 0;
//     self->mouse_y = 0;

//     self->frame_duration = 0.f;

//     self->owning_win = win == 0;
//     self->win = win == 0 ? createRenderWindow(windowTitle, createContextSettings()) : win;
//     self->renderTarget = new WindowRenderTarget(*this);
    
//     if (self->win != 0) {
//         self->win->SetTitle(windowTitle);
//         self->win->SetActive();



// #ifdef GLDEBUG
//         if (window().GetSettings().DebugContext)
//             glt::initDebug();
// #endif

//         window().EnableVerticalSync(false);
        
//         self->initialized = onInit();
//     }

// post_init:;

//     if (!self->initialized) {
//         delete self;
//         self = 0;

//         ERR("initialization failed");
//         return false;
//     }


//     float startupTime = self->clock->GetElapsedTime() - startupT0;

//     std::cerr << "successfully initialized in " << uint32(startupTime * 1000) << " ms" << std::endl;

//     return true;
// }

bool GameWindow::isKeyDown(sf::Key::Code key) {
    KeyState s = self->keyStates[int32(key)];
    return s != KEY_STATE_UP;
}

bool GameWindow::isButtonDown(sf::Mouse::Button button) {
    return self->mouseStates[int32(button)];
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

// void GameWindow::configureShaderVersion(glt::ShaderManager& mng) const {
//     const sf::ContextSettings& c = self->win->GetSettings();
//     glt::ShaderManager::ShaderProfile prof = c.CoreProfile ?
//         glt::ShaderManager::CoreProfile :
//         glt::ShaderManager::CompatibilityProfile;

//     int vers = c.MajorVersion * 100 + c.MinorVersion * 10;
//     mng.setShaderVersion(vers, prof);
// }

WindowRenderTarget& GameWindow::renderTarget(){
    return *self->renderTarget;
}

WindowEvents& GameWindow::events() {
    return self->events;
}

struct GameWindow::Data::HandleInput : public EventHandler<EngineEvent> {
    Data& win;
    HandleInput(Data& w) : win(w) {}    
    void handle(const Event<EngineEvent>&) {
        win.handleInputEvents();
    }
};

void GameWindow::registerHandlers(EngineEvents& evnts) {
    evnts.handleInput.registerHandler(glt::Ref<EventHandler<EngineEvent> >(new Data::HandleInput(*self)));
}

} // namespace ge
