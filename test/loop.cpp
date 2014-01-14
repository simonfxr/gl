#include "ge/GameLoop.hpp"
#include "sys/clock.hpp"
#include <stdio.h>

using namespace defs;

#define T 10 /* 10 secs */

struct Game : public ge::GameLoop::Game {

    ge::GameLoop loop;

    Game();
    virtual ~Game() OVERRIDE;
    virtual void tick() OVERRIDE;
    virtual void render(double interpolation) OVERRIDE;
    virtual void handleInputEvents() OVERRIDE;
    virtual ge::GameLoop::time now() OVERRIDE;
    virtual void sleep(ge::GameLoop::time seconds) OVERRIDE;
};

Game::Game() :
    loop(10000, 5, 20000)
{}

Game::~Game() {
}

void Game::tick() {
    if (loop.tickTime() > T)
        loop.exit(0);
}

void Game::render(double) {

}

void Game::handleInputEvents() {

}

ge::GameLoop::time Game::now() {
    return sys::queryTimer();
}

void Game::sleep(ge::GameLoop::time secs) {
    sys::sleep(secs);
}

int main() {
    Game game;


    double T0 = game.now();
    game.loop.run(game);
    double dur = game.now() - T0;

    printf("duration: %lf, avg ticks: %lf, avg draws: %lf\n", dur, game.loop.tickID() / dur, (game.loop.frameID() + 1) / dur);

    return 0;
}