#include "ge/GameLoop.hpp"
#include "err/err.hpp"

namespace ge {

using namespace defs;

struct GameLoop::Data {

    uint64 tick_id;
    uint64 frame_id;
    Game *game;

    time clock;
    time clock_offset;
    time tick_time;
    time tick_duration;
    time frame_duration;

    size max_skip;
    
    int32 exit_code;

    bool paused;
    bool sync_draw;
    bool stop;
    bool initialized;

    Data();
    time now();
};

GameLoop::time GameLoop::Data::now() {
    return game->now() - clock_offset;
}

GameLoop::Data::Data() :
    tick_id(0),
    frame_id(0),
    game(nullptr),
    clock(0),
    clock_offset(0),
    tick_time(0),
    tick_duration(0),
    frame_duration(0),
    max_skip(0),
    exit_code(0),
    paused(false),
    sync_draw(false),
    stop(false),
    initialized(false)
{}

GameLoop::GameLoop(defs::size _ticks, defs::size max_skip, defs::size max_fps) :
    self(new Data)
{
    ticks(_ticks);
    maxFramesSkipped(max_skip);
    maxFPS(max_fps);
}

GameLoop::~GameLoop() {
    delete self;
}

math::real GameLoop::tickTime() const {
    return math::real(self->tick_time);
}

math::real GameLoop::realTime() const {
    return math::real(self->clock);
}

math::real GameLoop::tickDuration() const {
    return math::real(self->frame_duration);
}

defs::size GameLoop::ticks() const {
    return defs::size(time(1) / self->frame_duration);
}

GameLoop& GameLoop::ticks(defs::size n) {
    self->tick_duration = time(1) / time(n);
    return *this;
}

defs::size GameLoop::maxFramesSkipped() const {
    return self->max_skip;
}

GameLoop& GameLoop::maxFramesSkipped(defs::size max_skip) {
    self->max_skip = max_skip;
    return *this;
}

defs::size GameLoop::maxFPS() const {
    return self->frame_duration == 0 ? 0 : size(1 / self->frame_duration);
}

GameLoop& GameLoop::maxFPS(defs::size max_fps) {
    self->frame_duration = (max_fps == 0 ? time(0) : time(1) / time(max_fps));
    return *this;
}

bool GameLoop::syncDraw() const {
    return self->sync_draw;
}

GameLoop& GameLoop::syncDraw(bool yesno) {
    self->sync_draw = yesno;
    return *this;
}

uint64 GameLoop::tickID() const {
    return self->tick_id;
}

uint64 GameLoop::frameID() const {
    return self->frame_id;
}

void GameLoop::exit(int32 exit_code) {
    self->exit_code = exit_code;
    self->stop = true;
}

GameLoop& GameLoop::pause(bool paused) {
    self->paused = paused;
    return *this;
}

bool GameLoop::paused() const {
    return self->paused;
}

int32 GameLoop::run(Game& logic) {
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

        size lim = self->sync_draw || self->max_skip == 0 ? 1 : self->max_skip;

        for (defs::index i = 0; ; ++i) {
            self->clock = self->now();
            auto cur_tick_duration = self->tick_duration;

            /*
             * if sync_draw, it may be that self->clock < next_tick,
             * make sure this doesnt happen
             */ 
            if ((!self->sync_draw || i > 0) &&
                ( self->clock < next_tick || i >= lim || self->stop) )
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
            interpolation = math::max(0, 1 - (next_tick - self->clock) / self->tick_duration);
        
        ASSERT(interpolation >= 0);
        ASSERT(interpolation <= 1);

        self->game->render(interpolation);
        ++self->frame_id;
        next_draw = (self->sync_draw ? next_tick : self->clock + self->frame_duration);

        self->clock = self->now();

        time diff = (next_tick < next_draw ? next_tick : next_draw) - self->clock;
        if (diff > 0)
            self->game->sleep(diff);
    }

    self->game->atExit(self->exit_code);

    self->game = nullptr;
    self->stop = false;
    return self->exit_code;
}

} // namespace ge
