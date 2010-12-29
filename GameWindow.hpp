#ifndef GAMEWINDOW_HPP
#define GAMEWINDOW_HPP

#include <string>

#include <SFML/Graphics.hpp>

#include "defs.h"
#include "GameLoop.hpp"


struct GameWindow {
private:

    struct Data;
    friend struct Data;

    GameLoop loop;
    Data *data;
    sf::RenderWindow *win;
    sf::Clock *clock;
    bool grab_mouse;
    
    uint64 game_frame_id;
    uint64 render_frame_id;

    bool change_pause_state;
    bool new_pause_state;
    
    bool init_successful;

    bool have_focus;

    GameWindow(const GameWindow& _);
    GameWindow& operator =(const GameWindow& _);

    virtual bool onInit();
    virtual void onExit();

    virtual void animate() = 0;
    virtual void renderScene(float interpolation) = 0;
    
    virtual void handleEvent(sf::Event& event);
    virtual void focusChanged(bool haveFocus);
    virtual void pauseStateChanged(bool isNowPaused);
    virtual void windowResized(uint32 width, uint32 height);
    virtual void keyStateChanged(sf::Key::Code key, bool pressed);
    virtual void mouseMoved(int32 dx, int32 dy);
    virtual void mouseButtonStateChanged(sf::Mouse::Button button, bool pressed);
    virtual void handleInternalEvents();

public:
    
    GameWindow();
    
    bool init(const sf::ContextSettings& settings, const std::string& name);
    
    virtual ~GameWindow();

    bool isKeyDown(sf::Key::Code key);
    
    bool isButtonDown(sf::Mouse::Button button);

    void requestExit(int32 exit_code = 0) { loop.exit(exit_code); }

    void grabMouse(bool grab = true) { grab_mouse = grab; }
    
    void pause(bool pauseGame = true)  {
        if (pauseGame != loop.paused()) {
            change_pause_state = true;
            new_pause_state = pauseGame;
        }
    }

    sf::RenderWindow& window() { return *win; }

    int32 run();

    void ticksPerSecond(uint32 ticks) { loop.ticks_per_second = ticks; }
    
    void maxFrameTicksSkip(uint32 ticks) { loop.max_frame_skip = ticks; }
    
    void maxFPS(uint32 fps) { loop.max_fps = fps; }

    float gameTime() { return loop.gameTime(); }
    
    float realTime() { return loop.realTime(); }

    uint64 currentFrameID() { return game_frame_id; }
    
    uint64 currentRenderFrameID() { return render_frame_id; }

    bool paused() { return change_pause_state ? new_pause_state : loop.paused(); }

    bool focused() { return have_focus; }

    float frameDuration() { return 1.f / loop.ticks_per_second; }
};

#endif
