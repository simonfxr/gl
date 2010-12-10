#include <iostream>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#include <GLBatch.h>

#include "defs.h"
#include "GameLoop.hpp"
#include "gltools.hpp"

namespace {

struct Game : public GameLoop::Game {

    sf::RenderWindow window;
    sf::Clock clock;
    GameLoop loop;

    Game(const sf::VideoMode& vm, const std::string& title, const sf::ContextSettings& cs);
    ~Game();

    void handleEvents();
    void tick();
    void render(float interpolation);
    float now();
    
};

Game::Game(const sf::VideoMode& vm, const std::string& title, const sf::ContextSettings& cs) :
    window(vm, title, sf::Style::Default, cs),
    loop(100, 10, 0)
{}

Game::~Game() {
    std::cerr << "exiting game after " << loop.realTime() << " seconds" << std::endl;
}

float Game::now() {
    return clock.GetElapsedTime();
}

void Game::handleEvents() {

    sf::Event e;

    while (window.GetEvent(e)) {
        switch (e.Type) {
        case sf::Event::Closed:
            loop.exit();
            break;
        }
    }
}

void Game::tick() {

}

void Game::render(float interpolation) {

    window.SetActive();

    glClearColor(0.f, 0.f, 0.f, 1.f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    window.Display();

    gltools::printErrors(std::cerr);
}

} // namespace anon

int main(int argc, char *argv[]) {

    sf::Clock startupClock;
    float t0 = startupClock.GetElapsedTime();

    sf::ContextSettings cs;
    cs.MajorVersion = 3;
    cs.MinorVersion = 3;
    Game game(sf::VideoMode(800, 600), "cube", cs);

    float t1 = startupClock.GetElapsedTime() - t0;

    std::cerr << "startup time: " << (t1 * 1000.f) << " ms" << std::endl;
        
    return game.loop.run(game);
}
