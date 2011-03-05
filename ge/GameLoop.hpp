#ifndef GAME_LOOP_HPP
#define GAME_LOOP_HPP

#include "defs.h"

namespace ge {

class GameLoop {
public:

    uint32 ticks_per_second;
    uint32 max_frame_skip;
    uint32 max_fps;

private:    

    bool _running;
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
        
        virtual void idle(float seconds) {
            UNUSED(seconds);
        }
    };
     
    explicit GameLoop(uint32 ticks_per_second, uint32 max_frame_skip = 0, uint32 max_fps = 0);

    float gameTime() const { return _now - _skipped_time - _start_time; }
    float realTime() const { return _now - _start_time; }
     
    void exit(int32 exit_code = 0);
    void pause(bool pause);

    bool paused() const;

    int32 run(Game& logic);
};

} // namespace ge

#endif
