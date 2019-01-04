#include "ge/GameLoop.hpp"
#include "sys/clock.hpp"
#include <cstdio>

#define T 10 /* 10 secs */

struct Game : public ge::GameLoop::Game
{

    ge::GameLoop loop;

    Game();
    ~Game() override;
    void tick() override;
    void render(double interpolation) override;
    void handleInputEvents() override;
    ge::GameLoop::time now() override;
    void sleep(ge::GameLoop::time secs) override;
};

Game::Game() : loop(10000, 5, 20000) {}

Game::~Game() = default;

void
Game::tick()
{
    if (loop.tickTime() > T)
        loop.exit(0);
}

void
Game::render(double /*interpolation*/)
{}

void
Game::handleInputEvents()
{}

ge::GameLoop::time
Game::now()
{
    return sys::queryTimer();
}

void
Game::sleep(ge::GameLoop::time secs)
{
    sys::sleep(secs);
}

int
main()
{
    Game game;

    double T0 = game.now();
    game.loop.run(game);
    double dur = double(game.now()) - T0;

    printf("duration: %lf, avg ticks: %lf, avg draws: %lf\n",
           dur,
           game.loop.tickID() / dur,
           (game.loop.frameID() + 1) / dur);

    return 0;
}
