#include "ge/GameLoop.hpp"

namespace ge {

GameLoop::GameLoop(uint32 _ticks_per_second, uint32 _max_frame_skip, uint32 _max_fps)
    : ticks_per_second(_ticks_per_second),
      max_frame_skip(_max_frame_skip),
      max_fps(_max_fps),
      _running(true),
      _paused(false),
      _now(0.f),
      _exit_code(0)
{}

void GameLoop::exit(int32 exit_code) {
    _running = false;
    _exit_code = exit_code;
}

int32 GameLoop::run(GameLoop::Game& logic) {

    const float tick_length      = 1.f / ticks_per_second;
    const float draw_tick_length = max_fps == 0 ? 0.f : 1.f / 0xFFFFFFFFUL;
    const uint32 loops_max       = max_frame_skip == 0 ? 0xFFFFFFFFUL : max_frame_skip;

    _start_time = logic.now();
    _skipped_time = 0.f;

    float next_game_tick = _start_time;
    float next_draw_tick = _start_time;

    while (_running) {
         
        uint32 loops = 0;
        
        while ((_now = logic.now()) >= next_game_tick && loops < loops_max && _running) {
            logic.handleInputEvents();

            if (unlikely(!_running))
                break;
            
            if (likely(!_paused))
                logic.tick();
            else
                _skipped_time += tick_length;
            
            next_game_tick += tick_length;
            ++loops;
        }

        if (unlikely(_now < next_draw_tick))
            continue;

        if (likely(!_paused)) {
            float interpolation = (_now + tick_length - next_game_tick) / tick_length;
            logic.render(interpolation);
        } else {
            logic.render(0.f);
        }

        next_draw_tick += draw_tick_length;
    }

    return _exit_code;
}

void GameLoop::pause(bool _pause) {
    _paused = _pause;
}

bool GameLoop::paused() const {
    return _paused;
}

} // namespace ge
