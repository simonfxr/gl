#include "ge/GameLoop.hpp"

namespace ge {

GameLoop::GameLoop(defs::size ticks_per_second, defs::size max_frame_skip, defs::size max_fps)
    : _ticks_per_second(ticks_per_second),
      _max_frame_skip(max_frame_skip),
      _max_fps(max_fps),
      _running(true),
      _exit(false),
      _restart(false),
      _paused(false),
      _sync(false),
      _now(0),
      _skipped_time(0),
      _start_time(0),
      _keepup_threshold(0.5f),
      _frame_duration(0),
      _exit_code(0),
      _animation_frame_id(0),
      _render_frame_id(0)      
{}

void GameLoop::exit(int32 exit_code) {
    _running = false;
    _restart = false;
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
        
        const float tick_length      = 1.f / float(_ticks_per_second);
        const float draw_tick_length = _sync || _max_fps == 0 ? 0.f : 1.f / float(_max_fps);
        const index loops_max       = _sync ? 1 : _max_frame_skip == 0 ? 0xFFFFFF : _max_frame_skip;
        const bool syncDraw = _sync;

        _frame_duration = tick_length;
        
        int running_behind = 0;

        bool reset_time = false;

        index loops = 0;

        _running = true;
        _restart = false;

        LoopState s1 = state;
        state = LoopBegin;

        switch (s1) {
        case LoopBegin: break;
        case LoopTick: goto tick;
        case LoopRender: goto render;
        }
            
        while (_running) {
            
            loops = 0;
            
            while ((_now = logic.now()) >= next_game_tick && _running) {

                if (loops >= loops_max) {
                    ++running_behind;
                    if (float(running_behind) * tick_length < _keepup_threshold)
                        goto behind;
                    reset_time = true;
                    break;
                }
                
                logic.handleInputEvents();
                    
                if (unlikely(!_running)) {
                    state = LoopTick;
                    goto out;
                }

            tick:

                if (likely(!_paused)) {
                    logic.tick();
                    ++_animation_frame_id;
                } else {
                    _skipped_time += tick_length;
                }
                
                next_game_tick += tick_length;
                ++loops;
            }

            running_behind = 0;

        behind:;

            if (unlikely(!_running)) {
                state = LoopRender;
                goto out;
            }

        render:

            if (unlikely(_now < next_draw_tick || (syncDraw && loops == 0))) {
                
                bool skip_rendering;
                float diff;
                
                if (!syncDraw && next_draw_tick < next_game_tick) {
                    skip_rendering = false;
                    diff = next_draw_tick - _now;
                } else {
                    skip_rendering = true;
                    diff = next_game_tick - _now;
                }

                if (diff > 0.f)
                    logic.sleep(diff);

                if (skip_rendering)
                    goto draw_skipped;
            }
            
            if (likely(!_paused && !syncDraw)) {
                float interpolation = (_now + tick_length - next_game_tick) / tick_length;
                logic.render(interpolation);
            } else {
                logic.render(0.f);
            }

            ++_render_frame_id;            
            next_draw_tick += draw_tick_length;

        draw_skipped:

            if (reset_time) {
                reset_time = false;
                float now = logic.now();
                float behind = now - next_game_tick;
                if (behind > 0.f) {
                    _skipped_time += behind;
                    next_game_tick = now;
                    next_draw_tick = now;
                }
            }
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
