#ifndef GAME_LOOP_HPP
#define GAME_LOOP_HPP

#include "defs.h"

typedef void (*TickF)(float );
typedef void (*DrawF)(float interpolation);
typedef void (*EventHandlerF)(void);
typedef float (*ClockF)();

class GameLoop {
public:
     
    const uint32 ticks_per_second;
    const uint32 max_frame_skip;
    const uint32 max_fps;

private:    

    bool _running;
    bool _paused; 
    float _now;
    float _skipped_time;

    int32 _exit_code;    

public:

    struct Game {
        virtual ~Game() {};
        virtual void tick() = 0;
        virtual void draw(float interpolation) = 0;
        virtual void handle_events() = 0;
        virtual float now() = 0;
    };
     
    GameLoop(uint32 ticks_per_second, uint32 max_frame_skip, uint32 max_fps = 0);

    float game_time() { return _now - _skipped_time; }
    float real_time() { return _now; }
     
    void exit(int32 exit_code = 0);
    void pause(bool pause);

    bool paused();

    int32 run(Game& logic);
};
#endif

