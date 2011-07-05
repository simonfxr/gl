#ifndef GAME_LOOP_HPP
#define GAME_LOOP_HPP

#include "defs.h"

namespace ge {

class GameLoop {
private:

    uint32 _ticks_per_second;
    uint32 _max_frame_skip;
    uint32 _max_fps;

    bool _running;
    bool _exit;
    bool _restart;
    bool _paused;
    bool _sync;
    
    float _now;
    float _skipped_time;
    float _start_time;

    float _keepup_threshold;

    float _frame_duration;

    int32 _exit_code;

    uint64 _animation_frame_id;
    uint64 _render_frame_id;

public:

    struct Game {
        virtual ~Game() {};
        virtual void tick() = 0;
        virtual void render(float interpolation) = 0;
        virtual void handleInputEvents() = 0;
        virtual float now() = 0;
        virtual void sleep(float seconds) { UNUSED(seconds); }
        virtual void exit(int32 exit_code) { UNUSED(exit_code); }
    };
     
    explicit GameLoop(uint32 ticks_per_second, uint32 max_frame_skip = 10, uint32 max_fps = 0);

    float gameTime() const { return _now - _skipped_time - _start_time; }
    
    float realTime() const { return _now - _start_time; }

    float frameDuration() const { return _frame_duration; }

    uint32 ticksPerSecond() const { return _ticks_per_second; }

    void ticksPerSecond(uint32 ticks) {
        _restart = true;
        _running = false;
        _ticks_per_second = ticks;
    }

    void maxDrawFramesSkipped(uint32 frames) {
        _restart = true;
        _running = false;
        _max_frame_skip = frames;
    }

    void maxFPS(uint32 max_fps) {
        _restart = true;
        _running = false;
        _max_fps = max_fps;
    }

    void updateKeepupThreshold(float secs) {
        _keepup_threshold = secs;
    }

    // synchronize drawing with simulation (draw every simulation frame exactly once)
    void sync(bool yesno) {
        _restart = true;
        _running = false;
        _sync = yesno;
    }

    uint64 renderFrameID() const { return _render_frame_id; }
    uint64 animationFrameID() const { return _animation_frame_id; }

    void exit(int32 exit_code = 0);
    
    void pause(bool pause);

    bool paused() const;

    int32 run(Game& logic);
};

} // namespace ge

#endif
