#ifndef GAME_LOOP_HPP
#define GAME_LOOP_HPP

#include "ge/conf.hpp"
#include "pp/pimpl.hpp"

#include <memory>

namespace ge {

struct GE_API GameLoop
{

    using time = double;

    struct GE_API Game
    {
        virtual ~Game();
        virtual void tick() = 0;
        virtual void render(double interpolation) = 0;
        virtual void handleInputEvents() = 0;
        virtual time now() const = 0;
        virtual void sleep(time seconds) = 0;
        virtual void atExit(int32_t exit_code) { UNUSED(exit_code); }
    };

    explicit GameLoop(size_t ticks, size_t max_skip = 10, size_t max_fps = 120);

    HU_NODISCARD time tickTime() const;

    HU_NODISCARD time realTime() const;

    HU_NODISCARD time tickDuration() const;

    HU_NODISCARD size_t ticks() const;
    GameLoop &ticks(size_t n);

    HU_NODISCARD size_t maxFramesSkipped() const;
    GameLoop &maxFramesSkipped(size_t max_skip);

    HU_NODISCARD size_t maxFPS() const;
    GameLoop &maxFPS(size_t max_fps);

    HU_NODISCARD bool syncDraw() const;
    GameLoop &syncDraw(bool yesno);

    HU_NODISCARD uint64_t tickID() const;
    HU_NODISCARD uint64_t frameID() const;

    HU_NODISCARD bool paused() const;
    GameLoop &pause(bool pause = true);

    void exit(int32_t exit_code = 0);

    int32_t run(Game &logic);

private:
    DECLARE_PIMPL(GE_API, self);
};

} // namespace ge

#endif
