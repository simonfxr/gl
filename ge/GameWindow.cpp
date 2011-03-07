#include <vector>
#include <limits>
#include <iostream>

#include <GL/glew.h>

#include "defs.h"

#include "glt/utils.hpp"

#include "ge/GameWindow.hpp"

#ifdef SYSTEM_UNIX
#define HAVE_UNISTD
#include <unistd.h>
#endif

namespace ge {

struct KeyState {
    float keyAge;

    KeyState() : keyAge(1.f / 0.f) {}
    
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

struct GameWindow::Data : public GameLoop::Game {
    GameWindow &self;

    bool initialized;
    
    GameLoop loop;
    
    bool owning_win;
    sf::RenderWindow *win;
    
    bool owning_clock;
    sf::Clock *clock;
    
    bool grab_mouse;
    uint64 game_frame_id;
    uint64 render_frame_id;

    bool change_pause_state;
    bool new_pause_state;
    
    bool have_focus;
    
    KeyState keyStates[sf::Key::Count];
    bool mouseStates[sf::Mouse::ButtonCount];
    
    int32 mouse_x;
    int32 mouse_y;

    float frame_duration;

    Data(GameWindow& _self) :
        self(_self), loop(30) {}
    
    ~Data();

    float now();
    void handleInputEvents();
    void tick();
    void render(float interpolation);
    void sleep(float time);
};

GameWindow::Data::~Data() {
    if (owning_win) {
        delete win;
        win = 0;
    }

    if (owning_clock) {
        delete clock;
        clock = 0;
    }
}

#define SELF ({                                                         \
    ASSERT(self != 0, "self == NULL (not initialized?)");               \
    self;                                                               \
        })

GameWindow::GameWindow() :
    self(0) {}
    
GameWindow::~GameWindow() {
    if (self != 0) {
        delete self;
        self = 0;
    }
}

float GameWindow::Data::now() {
    return clock->GetElapsedTime();
}

void GameWindow::Data::handleInputEvents() {

    frame_duration = 1.f / loop.ticksPerSecond();

    int32 mouse_current_x = mouse_x;
    int32 mouse_current_y = mouse_y;

    bool was_resize = false;
    uint32 new_w = 0;
    uint32 new_h = 0;

    sf::Event e;
    while (win->GetEvent(e)) {

        if (change_pause_state) {
            change_pause_state = false;
            loop.pause(new_pause_state);
            self.pauseStateChanged(new_pause_state);
        }
        
        switch (e.Type) {
        case sf::Event::Closed:
            self.requestExit();
            break;
            
        case sf::Event::Resized:
            was_resize = true;
            new_w = e.Size.Width;
            new_h = e.Size.Height;
            break;
            
        case sf::Event::KeyPressed:
            keyStates[int32(e.Key.Code)] = KeyState(self.realTime());
            self.keyStateChanged(e.Key, true);
            break;
            
        case sf::Event::KeyReleased:
            keyStates[int32(e.Key.Code)] = KEY_STATE_UP;
            self.keyStateChanged(e.Key, false);
            break;
            
        case sf::Event::MouseMoved:
            mouse_x = e.MouseMove.X;
            mouse_y = e.MouseMove.Y;
            break;
            
        case sf::Event::MouseButtonPressed:
            mouse_x = e.MouseButton.X;
            mouse_y = e.MouseButton.Y;
            mouseStates[int32(e.MouseButton.Button)] = true;
            self.mouseButtonStateChanged(e.MouseButton, true);
            break;

        case sf::Event::MouseButtonReleased:
            mouse_x = e.MouseButton.X;
            mouse_y = e.MouseButton.Y;
            mouseStates[int32(e.MouseButton.Button)] = false;
            self.mouseButtonStateChanged(e.MouseButton, false);
            break;
            
        case sf::Event::LostFocus:
            
            if (have_focus) {
                have_focus = false;
                win->ShowMouseCursor(true);
                self.focusChanged(false);
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
            
                self.focusChanged(true);
            }
            
            break;

        default:
            self.handleInputEvent(e);
            break;
        }
    }

    if (was_resize) {

        if (grab_mouse) {
            mouse_current_x = mouse_x = new_w / 2;
            mouse_current_y = mouse_y = new_h / 2;
            win->SetCursorPosition(mouse_x, mouse_y);
        }
        
        self.windowResized(new_w, new_h);
    } else {

        int32 dx = mouse_current_x - mouse_x;
        int32 dy = mouse_y - mouse_current_y;

        if (have_focus && (dx != 0 || dy != 0)) {
            if (grab_mouse) {
                mouse_x = win->GetWidth() / 2;
                mouse_y = win->GetHeight() / 2;
                win->SetCursorPosition(mouse_x, mouse_y);
            }
            self.mouseMoved(dx, dy);
        }
    }

    self.handleInternalEvents();
}

void GameWindow::Data::tick() {
    self.animate();
    ++game_frame_id;
}

void GameWindow::Data::render(float interpolation) {
    self.renderScene(interpolation);
    ++render_frame_id;
    win->Display();
}

void GameWindow::Data::sleep(float time) {
#ifdef HAVE_UNISTD
    usleep(uint32(time * 1000000));
#else
    UNUSED(time);
#endif
}

bool GameWindow::onInit() {
    return true;
}

sf::RenderWindow *GameWindow::createRenderWindow(const std::string& title) {

    sf::ContextSettings glContext;
    glContext.MajorVersion = 3;
    glContext.MinorVersion = 3;
    glContext.AntialiasingLevel = 0;
#ifdef GLDEBUG
    glContext.DebugContext = true;
#endif

    return new sf::RenderWindow(sf::VideoMode(800, 600), title, sf::Style::Default, glContext);
}

void GameWindow::onExit(int32 exit_code) {
    UNUSED(exit_code);
}


bool GameWindow::init(const std::string& windowTitle, sf::RenderWindow *win, sf::Clock *clock) {

    if (self != 0) {
        ERROR("GameWindow already initialized");
        return false;
    }

    self = new Data(*this);

    if (clock == 0) {
        self->owning_clock = true;
        self->clock = new sf::Clock;
    }

    float startupT0 = self->clock->GetElapsedTime();

    self->initialized = false;
    self->grab_mouse = false;
    self->game_frame_id = 0;
    self->render_frame_id = 0;
    self->change_pause_state = false;
    self->new_pause_state = false;
    self->have_focus = true;

    for (uint32 i = 0; i < ARRAY_LENGTH(self->keyStates); ++i)
        self->keyStates[i] = KEY_STATE_UP;

    for (uint32 i = 0; i < ARRAY_LENGTH(self->mouseStates); ++i)
        self->mouseStates[i] = false;

    self->mouse_x = 0;
    self->mouse_y = 0;

    self->frame_duration = 0.f;

    if (win == 0) {
        self->owning_win = true;
        self->win = createRenderWindow(windowTitle);
    } else {
        self->owning_win = false;
        self->win = win;
    }
    
    if (self->win != 0) {
        self->win->SetActive();

        const sf::ContextSettings& c = self->win->GetSettings();

        std::cerr << "Initialized OpenGL Context"<< std::endl
                  << "  Version:\t" << c.MajorVersion << "." << c.MinorVersion << std::endl
                  << "  DepthBits:\t" << c.DepthBits << std::endl
                  << "  StencilBits:\t" << c.StencilBits << std::endl
                  << "  Antialiasing:\t" << c.AntialiasingLevel << std::endl
#ifdef GLDEBUG
                  << "  DebugContext:\t" << (c.DebugContext ? "yes" : "no") << std::endl
#endif
                  << std::endl;

        GLenum err = glewInit();
        if (GLEW_OK != err) {
            std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
            goto post_init;
        }

#ifdef GLDEBUG
        if (window().GetSettings().DebugContext)
            glt::initDebug();
#endif

        window().EnableVerticalSync(false);
        
        self->initialized = onInit();
    }

post_init:;

    if (!self->initialized) {
        delete self;
        self = 0;

        ERROR("initialization failed");
        return false;
    }

    if (self->grab_mouse)
        self->win->ShowMouseCursor(false);
    
    self->win->EnableKeyRepeat(false);

    float startupTime = self->clock->GetElapsedTime() - startupT0;

    std::cerr << "successfully initialized in " << uint32(startupTime * 1000) << " ms" << std::endl;

    return true;
}

bool GameWindow::isKeyDown(sf::Key::Code key) {
    KeyState s = SELF->keyStates[int32(key)];
    return s != KEY_STATE_UP;
}

bool GameWindow::isButtonDown(sf::Mouse::Button button) {
    return self->mouseStates[int32(button)];
}

int32 GameWindow::run() {
    
    if (self == 0 || !self->initialized) {
        ERROR("GameWindow not initialized");
        return -1;
    }
    
    int32 e = self->loop.run(*self);
    onExit(e);
    
    return e;
}

float GameWindow::now() const {
    return SELF->now();
}

void GameWindow::handleInputEvent(sf::Event& event) {
    UNUSED(event);
}

void GameWindow::focusChanged(bool haveFocus) {
    UNUSED(haveFocus);
}

void GameWindow::pauseStateChanged(bool isNowPaused) {
    UNUSED(isNowPaused);
}

void GameWindow::windowResized(uint32 width, uint32 height) {
    UNUSED(width); UNUSED(height);
}

void GameWindow::keyStateChanged(const sf::Event::KeyEvent& key, bool pressed) {
    UNUSED(key); UNUSED(pressed);
}

void GameWindow::mouseMoved(int32 dx, int32 dy) {
    UNUSED(dx); UNUSED(dy);
}

void GameWindow::mouseButtonStateChanged(const sf::Event::MouseButtonEvent& button, bool pressed) {
    UNUSED(button); UNUSED(pressed);
}

void GameWindow::handleInternalEvents() {

}

float GameWindow::gameTime() const {
    return SELF->loop.gameTime();
}
    
float GameWindow::realTime() const {
    return SELF->loop.realTime();
}
    
void GameWindow::requestExit(int32 exit_code) {
    SELF->loop.exit(exit_code);
}

void GameWindow::grabMouse(bool grab) {
    SELF->grab_mouse = grab;
}
    
void GameWindow::pause(bool pauseGame)  {
    UNUSED(SELF);
    if (pauseGame != paused()) {
        SELF->change_pause_state = true;
        SELF->new_pause_state = pauseGame;
    }
}

sf::RenderWindow& GameWindow::window() {
    return *SELF->win;
}

void GameWindow::ticksPerSecond(uint32 ticks) { 
    SELF->loop.updateTicksPerSecond(ticks);
}

void GameWindow::maxDrawFramesSkipped(uint32 ticks) {
    SELF->loop.updateMaxDrawFramesSkipped(ticks);
}
    
void GameWindow::maxFPS(uint32 fps) {
    SELF->loop.updateMaxFPS(fps);
}

uint64 GameWindow::currentFrameID() const {
    return SELF->game_frame_id;
}

uint64 GameWindow::currentRenderFrameID() const {
    return SELF->render_frame_id;
}

bool GameWindow::paused() const {
    return SELF->change_pause_state ? SELF->new_pause_state : SELF->loop.paused();
}

bool GameWindow::focused() const {
    return SELF->have_focus;
}

float GameWindow::frameDuration() const {
    return SELF->frame_duration;
}

} // namespace ge
