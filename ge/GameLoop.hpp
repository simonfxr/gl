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
    float _now;
    float _skipped_time;
    float _start_time;

    int32 _exit_code;    

public:

    struct Game {
        virtual ~Game() {};
        virtual void tick() = 0;
        virtual void render(float interpolation) = 0;
        virtual void handleInputEvents() = 0;
        virtual float now() = 0;
        virtual void sleep(float seconds) {
            UNUSED(seconds);
        }
    };
     
    explicit GameLoop(uint32 ticks_per_second, uint32 max_frame_skip = 10, uint32 max_fps = 0);

    float gameTime() const { return _now - _skipped_time - _start_time; }
    
    float realTime() const { return _now - _start_time; }

    uint32 ticksPerSecond() const { return _ticks_per_second; }

    void updateTicksPerSecond(uint32 ticks) {
        _restart = true;
        _running = false;
        _ticks_per_second = ticks;
    }

    void updateMaxDrawFramesSkipped(uint32 frames) {
        _restart = true;
        _running = false;
        _max_frame_skip = frames;
    }

    void updateMaxFPS(uint32 max_fps) {
        _restart = true;
        _running = false;
        _max_fps = max_fps;
    }

    void exit(int32 exit_code = 0);
    
    void pause(bool pause);

    bool paused() const;

    int32 run(Game& logic);
};

} // namespace ge

#endif
