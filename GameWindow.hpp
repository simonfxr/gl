#ifndef GAMEWINDOW_HPP
#define GAMEWINDOW_HPP

#include <SFML/Graphics.hpp>

#include "defs.h"
#include "GameLoop.hpp"

struct GameWindow {

    bool init(const sf::ContextSettings& settings);
    bool isKeyDown(sf::Key::Code key);

    void requestExit(int32 exit_code = 0);

    float gameTime();
    float realTime();

    void grabMouse(bool enable = true);
    void pause(bool pauseGame = true);
    
    sf::RenderWindow& renderWindow();

    int32 run();

    uint64 currentFrameID();
    uint64 currentRenderFrameID();

private:

    struct Game;
    friend struct Game;

    GameLoop loop;
    Game *game;
    sf::RenderWindow *win;
    sf::Clock *clock;
    bool grab_mouse;
    
    uint64 game_frame_id;
    uint64 render_frame_id;

    virtual bool onInit();
    virtual bool onExit();
    
    virtual void handleEvent(sf::Event& event);
    virtual void focusChanged(bool haveFocus);
    virtual void keyPressed(sf::Key::Code key);
    virtual void mouseMoved(int32 dx, int32 dy);
    virtual void mousePressed(sf::Mouse::Button button);
    virtual void renderScene(float interpolation) = 0;
    virtual void animate() = 0;
};

#endif

