#ifndef GAMEWINDOW_HPP
#define GAMEWINDOW_HPP

#include <string>

#include <SFML/Graphics.hpp>

#include "defs.h"
#include "ge/GameLoop.hpp"
#include "ge/WindowRenderTarget.hpp"
#include "glt/ShaderManager.hpp"

namespace ge {

struct GameWindow {
    GameWindow();
    
    virtual ~GameWindow();

    float now() const;

    float gameTime() const;
    
    float realTime() const;

    bool init(const std::string& windowTitle, sf::RenderWindow *win = 0, sf::Clock *clock = 0);

    bool isKeyDown(sf::Key::Code key);
    
    bool isButtonDown(sf::Mouse::Button button);

    void requestExit(int32 exit_code = 0);

    void grabMouse(bool grab = true);
    
    void pause(bool pauseGame = true);
    
    sf::RenderWindow& window();

    int32 run();

    void ticksPerSecond(uint32 ticks);
    
    void maxDrawFramesSkipped(uint32 ticks);
    
    void maxFPS(uint32 fps);

    void synchronizeDrawing(bool sync);

    uint64 currentFrameID() const;
    
    uint64 currentRenderFrameID() const;

    bool paused() const;

    bool focused() const;

    float frameDuration() const;

    void configureShaderVersion(glt::ShaderManager& mng) const;

    WindowRenderTarget& renderTarget();

private:
    
    GameWindow(const GameWindow& _);
    GameWindow& operator =(const GameWindow& _);

    virtual bool onInit();

    virtual sf::ContextSettings createContextSettings();

    virtual sf::RenderWindow *createRenderWindow(const std::string& windowTitle, const sf::ContextSettings& ctxconf);
    
    virtual void onExit(int32 exit_code);

    virtual void animate() = 0;
    
    virtual void renderScene(float interpolation) = 0;
    
    virtual void handleInputEvent(sf::Event& event);
    
    virtual void focusChanged(bool haveFocus);
    
    virtual void pauseStateChanged(bool isNowPaused);
    
    virtual void windowResized(uint32 width, uint32 height);
    
    virtual void keyStateChanged(const sf::Event::KeyEvent& key, bool pressed);
    
    virtual void mouseMoved(int32 dx, int32 dy);
    
    virtual void mouseButtonStateChanged(const sf::Event::MouseButtonEvent& button, bool pressed);
    
    virtual void handleInternalEvents();

    struct Data;
    Data *self;
};

} // namespace ge

#endif
