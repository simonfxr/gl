#include "ge/GameLoop.hpp"

namespace ge {

GameLoop::GameLoop(uint32 ticks_per_second, uint32 max_frame_skip, uint32 max_fps)
    : _ticks_per_second(ticks_per_second),
      _max_frame_skip(max_frame_skip),
      _max_fps(max_fps),
      _running(true),
      _exit(false),
      _restart(false),
      _paused(false),
      _now(0.f),
      _exit_code(0)
{}

void GameLoop::exit(int32 exit_code) {
    _running = false;
    _exit = true;
    _exit_code = exit_code;
}

int32 GameLoop::run(GameLoop::Game& logic) {
    
    _start_time = logic.now();
    _skipped_time = 0.f;
    _exit = false;

    float next_game_tick = _start_time;
    float next_draw_tick = _start_time;

    enum LoopState {
        LoopBegin,
        LoopTick,
        LoopRender
    };

    LoopState state = LoopBegin;

    do {
        
        const float tick_length      = 1.f / _ticks_per_second;
        const float draw_tick_length = _max_fps == 0 ? 0.f : 1.f / _max_fps;
        const uint32 loops_max       = _max_frame_skip == 0 ? 0xFFFFFFFFUL : _max_frame_skip;
        uint32 loops = 0;

        _running = true;
        _restart = false;

        switch (state) {
        case LoopBegin: break;
        case LoopTick: goto tick;
        case LoopRender: goto render;
        }
            
        while (_running) {
            
            loops = 0;
            
            while ((_now = logic.now()) >= next_game_tick && loops < loops_max && _running) {
                logic.handleInputEvents();
                    
                if (unlikely(!_running)) {
                    state = LoopTick;
                    goto out;
                }

            tick:

                if (likely(!_paused))
                    logic.tick();
                else
                    _skipped_time += tick_length;
                
                next_game_tick += tick_length;
                ++loops;
            }

            if (unlikely(!_running)) {
                state = LoopRender;
                goto out;
            }

        render:

            if (unlikely(_now < next_draw_tick)) {
                
                bool skip_rendering;
                float diff;
                
                if (next_draw_tick < next_game_tick) {
                    skip_rendering = false;
                    diff = next_draw_tick - _now;
                } else {
                    skip_rendering = true;
                    diff = next_game_tick - _now;
                }

                if (diff > 0.001f)
                    logic.sleep(diff - 0.001f);

                if (skip_rendering)
                    continue;
            }
            
            if (likely(!_paused)) {
                float interpolation = (_now + tick_length - next_game_tick) / tick_length;
                logic.render(interpolation);
            } else {
                logic.render(0.f);
            }
            
            next_draw_tick += draw_tick_length;
        }

    out:;
        
    } while (!_exit && _restart);

    return _exit_code;
}

void GameLoop::pause(bool _pause) {
    _paused = _pause;
}

bool GameLoop::paused() const {
    return _paused;
}

} // namespace ge
