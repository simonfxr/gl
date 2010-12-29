#include <vector>
// #include <iostream>
#include <limits>

#include "GameWindow.hpp"

struct GameWindow::Data : public GameLoop::Game {
    GameWindow &self;

    std::vector<float> keyStates;
    std::vector<bool> mouseStates;
    
    int32 mouse_x;
    int32 mouse_y;

    Data(GameWindow& _self) :
        self(_self),
        keyStates(sf::Key::Count, std::numeric_limits<float>::infinity()),
        mouseStates(sf::Mouse::ButtonCount, false),
        mouse_x(0),
        mouse_y(0)
        {}

    float now();
    void handleEvents();
    void tick();
    void render(float interpolation);
};

float GameWindow::Data::now() {
    return self.clock->GetElapsedTime();
}

void GameWindow::Data::handleEvents() {

    int32 mouse_current_x = mouse_x;
    int32 mouse_current_y = mouse_y;

    bool was_resize = false;
    uint32 new_w = 0;
    uint32 new_h = 0;

    uint32 loops = 0;
    
    sf::Event e;
    while (self.win->GetEvent(e)) {

        ++loops;
        
        if (self.change_pause_state) {
            self.change_pause_state = false;
            self.loop.pause(self.new_pause_state);
            self.pauseStateChanged(self.new_pause_state);
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
            keyStates[int32(e.Key.Code)] = self.realTime();
            self.keyStateChanged(e.Key.Code, true);
            break;
            
        case sf::Event::KeyReleased:
            keyStates[int32(e.Key.Code)] = std::numeric_limits<float>::infinity();
            self.keyStateChanged(e.Key.Code, false);
            break;
            
        case sf::Event::MouseMoved:
            mouse_x = e.MouseMove.X;
            mouse_y = e.MouseMove.Y;
            break;
            
        case sf::Event::MouseButtonPressed:
            mouse_x = e.MouseButton.X;
            mouse_y = e.MouseButton.Y;
            self.mouseButtonStateChanged(e.MouseButton.Button, true);
            break;

        case sf::Event::MouseButtonReleased:
            mouse_x = e.MouseButton.X;
            mouse_y = e.MouseButton.Y;
            self.mouseButtonStateChanged(e.MouseButton.Button, false);
            break;
            
        case sf::Event::LostFocus:
            
            if (self.have_focus) {
                self.have_focus = false;
                self.win->ShowMouseCursor(true);
                self.focusChanged(false);
            }
            
            break;
            
        case sf::Event::GainedFocus:

            if (!self.have_focus) {
                self.have_focus = true;
            
                if (self.grab_mouse) {
                    self.win->ShowMouseCursor(false);

                    uint32 win_w = self.win->GetWidth();
                    uint32 win_h = self.win->GetHeight();
                
                    mouse_current_x = mouse_x = win_w / 2;
                    mouse_current_y = mouse_y = win_h / 2;

                    self.win->SetCursorPosition(mouse_x, mouse_y);
                }
            
                self.focusChanged(true);
            }
            
            break;

        default:
            self.handleEvent(e);
            break;
        }
    }

    if (was_resize) {

        mouse_current_x = mouse_x = new_w / 2;
        mouse_current_y = mouse_y = new_h / 2;
        self.win->SetCursorPosition(mouse_x, mouse_y);
        self.windowResized(new_w, new_h);
        
    } else {

        int32 dx = mouse_current_x - mouse_x;
        int32 dy = mouse_y - mouse_current_y;

        if (self.have_focus && (dx != 0 || dy != 0)) {
            if (self.grab_mouse) {
                mouse_x = self.win->GetWidth() / 2;
                mouse_y = self.win->GetHeight() / 2;
                self.win->SetCursorPosition(mouse_x, mouse_y);
            }
            self.mouseMoved(dx, dy);
        }
    }

    // if (loops != 0)
    //     std::cerr << "event loop: processed " << loops << " events" << std::endl;

    self.handleInternalEvents();
}

void GameWindow::Data::tick() {
    self.animate();
    ++self.game_frame_id;
}

void GameWindow::Data::render(float interpolation) {
    self.renderScene(interpolation);
    ++self.render_frame_id;
    self.win->Display();
}

GameWindow::GameWindow() :
    loop(0),
    data(0),
    win(0),
    clock(0),
    grab_mouse(true),
    game_frame_id(0),
    render_frame_id(0),
    change_pause_state(false),
    new_pause_state(false),
    init_successful(false),
    have_focus(true)
{}

GameWindow::~GameWindow() {
    delete data;
    delete win;
    delete clock;
}

bool GameWindow::onInit() {
    return true;
}

void GameWindow::onExit() {

}

bool GameWindow::init(const sf::ContextSettings& settings, const std::string& name) {

    if (init_successful)
        return true;

    data = new Data(*this);
    win = new sf::RenderWindow(sf::VideoMode(800, 600), name, sf::Style::Default, settings);
    clock = new sf::Clock();
    win->EnableKeyRepeat(false);
    
    init_successful = onInit();

    if (!init_successful) {
        
        delete data; data = 0;
        delete win; win = 0;
        delete clock; clock = 0;
        
    } else {

        if (grab_mouse)
            win->ShowMouseCursor(false);
    }

    return init_successful;
}

bool GameWindow::isKeyDown(sf::Key::Code key) {
    float keyAge = realTime() - data->keyStates[int32(key)];
    return keyAge == 0.f || keyAge >= 0.035f;
}

bool GameWindow::isButtonDown(sf::Mouse::Button button) {
    return data->mouseStates[int32(button)];
}

int32 GameWindow::run() {
    if (!init_successful)
        return -1;
    int32 e = loop.run(*data);
    onExit();
    return e;
}

void GameWindow::handleEvent(sf::Event& event) {
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

void GameWindow::keyStateChanged(sf::Key::Code key, bool pressed) {
    UNUSED(key); UNUSED(pressed);
}

void GameWindow::mouseMoved(int32 dx, int32 dy) {
    UNUSED(dx); UNUSED(dy);
}

void GameWindow::mouseButtonStateChanged(sf::Mouse::Button button, bool pressed) {
    UNUSED(button); UNUSED(pressed);
}

void GameWindow::handleInternalEvents() {

}
