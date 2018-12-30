#ifndef GAME_LOOP_HPP
#define GAME_LOOP_HPP

#include "ge/conf.hpp"
#include "math/real.hpp"

namespace ge {

using namespace defs;

struct GE_API GameLoop
{

    typedef math::real time;

    struct Game
    {
        virtual ~Game();
        virtual void tick() = 0;
        virtual void render(double interpolation) = 0;
        virtual void handleInputEvents() = 0;
        virtual time now() = 0;
        virtual void sleep(time seconds) = 0;
        virtual void atExit(int32 exit_code) { UNUSED(exit_code); }
    };

    explicit GameLoop(defs::size ticks,
                      defs::size max_skip = 10,
                      defs::size max_fps = 120);
    ~GameLoop();

    time tickTime() const;

    time realTime() const;

    time tickDuration() const;

    defs::size ticks() const;
    GameLoop &ticks(defs::size n);

    defs::size maxFramesSkipped() const;
    GameLoop &maxFramesSkipped(defs::size max_skip);

    defs::size maxFPS() const;
    GameLoop &maxFPS(defs::size max_fps);

    bool syncDraw() const;
    GameLoop &syncDraw(bool yesno);

    uint64 tickID() const;
    uint64 frameID() const;

    bool paused() const;
    GameLoop &pause(bool pause = true);

    void exit(int32 exit_code = 0);

    int32 run(Game &logic);

private:
private:
    GameLoop(const GameLoop &);
    GameLoop &operator=(const GameLoop &);

    struct Data;
    Data *const self;
};

} // namespace ge

#endif
