#ifndef GAME_LOOP_HPP
#define GAME_LOOP_HPP

#include "ge/conf.hpp"

#include <memory>

namespace ge {

struct GE_API GameLoop
{

    typedef double time;

    struct GE_API Game
    {
        virtual ~Game();
        virtual void tick() = 0;
        virtual void render(double interpolation) = 0;
        virtual void handleInputEvents() = 0;
        virtual time now() = 0;
        virtual void sleep(time seconds) = 0;
        virtual void atExit(int32_t exit_code) { UNUSED(exit_code); }
    };

    explicit GameLoop(size_t ticks, size_t max_skip = 10, size_t max_fps = 120);

    time tickTime() const;

    time realTime() const;

    time tickDuration() const;

    size_t ticks() const;
    GameLoop &ticks(size_t n);

    size_t maxFramesSkipped() const;
    GameLoop &maxFramesSkipped(size_t max_skip);

    size_t maxFPS() const;
    GameLoop &maxFPS(size_t max_fps);

    bool syncDraw() const;
    GameLoop &syncDraw(bool yesno);

    uint64_t tickID() const;
    uint64_t frameID() const;

    bool paused() const;
    GameLoop &pause(bool pause = true);

    void exit(int32_t exit_code = 0);

    int32_t run(Game &logic);

private:
    DECLARE_PIMPL(GE_API, self);
};

} // namespace ge

#endif
