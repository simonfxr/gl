#include "ge/GameLoop.hpp"

#include "data/range.hpp"
#include "err/err.hpp"

#include <algorithm>

namespace ge {

GameLoop::Game::~Game() = default;

struct GameLoop::Data
{

    uint64_t tick_id{};
    uint64_t frame_id{};
    Game *game{ nullptr };

    time clock{};
    time clock_offset{};
    time tick_time{};
    time tick_duration{};
    time frame_duration{};

    size_t max_skip{};

    int32_t exit_code{};

    bool paused{ false };
    bool sync_draw{ false };
    bool stop{ false };
    bool initialized{ false };

    Data();
    time now();
};

DECLARE_PIMPL_DEL(GameLoop)

GameLoop::time
GameLoop::Data::now()
{
    return game->now() - clock_offset;
}

GameLoop::Data::Data() = default;

GameLoop::GameLoop(size_t _ticks, size_t max_skip, size_t max_fps)
  : self(new Data)
{
    ticks(_ticks);
    maxFramesSkipped(max_skip);
    maxFPS(max_fps);
}

GameLoop::time
GameLoop::tickTime() const
{
    return self->tick_time;
}

GameLoop::time
GameLoop::realTime() const
{
    return self->clock;
}
GameLoop::time
GameLoop::tickDuration() const
{
    return self->frame_duration;
}

size_t
GameLoop::ticks() const
{
    return size_t(time(1) / self->frame_duration);
}

GameLoop &
GameLoop::ticks(size_t n)
{
    self->tick_duration = time(1) / time(n);
    return *this;
}

size_t
GameLoop::maxFramesSkipped() const
{
    return self->max_skip;
}

GameLoop &
GameLoop::maxFramesSkipped(size_t max_skip)
{
    self->max_skip = max_skip;
    return *this;
}

size_t
GameLoop::maxFPS() const
{
    return self->frame_duration == 0 ? 0 : size_t(1 / self->frame_duration);
}

GameLoop &
GameLoop::maxFPS(size_t max_fps)
{
    self->frame_duration = (max_fps == 0 ? time(0) : time(1) / time(max_fps));
    return *this;
}

bool
GameLoop::syncDraw() const
{
    return self->sync_draw;
}

GameLoop &
GameLoop::syncDraw(bool yesno)
{
    self->sync_draw = yesno;
    return *this;
}

uint64_t
GameLoop::tickID() const
{
    return self->tick_id;
}

uint64_t
GameLoop::frameID() const
{
    return self->frame_id;
}

void
GameLoop::exit(int32_t exit_code)
{
    self->exit_code = exit_code;
    self->stop = true;
}

GameLoop &
GameLoop::pause(bool paused)
{
    self->paused = paused;
    return *this;
}

bool
GameLoop::paused() const
{
    return self->paused;
}

int32_t
GameLoop::run(Game &logic)
{
    self->game = &logic;

    time start_time = self->game->now();
    time next_tick = self->clock;
    time next_draw = self->clock;

    if (!self->initialized) {
        self->initialized = true;
        self->clock_offset = start_time;
    } else {
        self->clock_offset = start_time - self->clock;
    }

    while (!self->stop) {

        size_t lim =
          self->sync_draw || self->max_skip == 0 ? 1 : self->max_skip;

        for (const auto i : irange()) {
            self->clock = self->now();
            auto cur_tick_duration = self->tick_duration;

            /*
             * if sync_draw, it may be that self->clock < next_tick,
             * make sure this doesnt happen
             */
            if ((!self->sync_draw || i > 0) &&
                (self->clock < next_tick || i >= lim || self->stop))
                break;

            self->game->handleInputEvents();
            if (self->stop)
                break;

            if (!self->paused) {
                self->game->tick();
                ++self->tick_id;
                self->tick_time += cur_tick_duration;
            }

            next_tick += cur_tick_duration;
        }

        if (self->stop)
            break;

        double interpolation;
        if (self->sync_draw || self->paused || self->clock >= next_tick)
            interpolation = 0;
        else
            interpolation = std::max(
              time(0), 1 - (next_tick - self->clock) / self->tick_duration);

        ASSERT(interpolation >= 0);
        ASSERT(interpolation <= 1);

        self->game->render(interpolation);
        ++self->frame_id;
        next_draw =
          (self->sync_draw ? next_tick : self->clock + self->frame_duration);

        self->clock = self->now();

        time diff =
          (next_tick < next_draw ? next_tick : next_draw) - self->clock;
        if (diff > 0)
            self->game->sleep(diff);
    }

    self->game->atExit(self->exit_code);

    self->game = nullptr;
    self->stop = false;
    return self->exit_code;
}

} // namespace ge
