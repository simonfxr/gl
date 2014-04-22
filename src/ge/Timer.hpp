#ifndef GE_TIMER_HPP
#define GE_TIMER_HPP

#include "ge/conf.hpp"

namespace ge {

struct Engine;

struct Timer {

    enum State {
        Inactive,
        Active,
        ActiveRepeated
    };
    
    Engine& engine;
    math::real alarm;
    math::real countdown;
    State state;
    
    Timer(Engine& e) : engine(e), alarm(0.f), countdown(0.f), state(Inactive) {}
    
    void start(math::real countdown_secs, bool repeat = false) {
        countdown = countdown_secs;
        alarm = engine.now() + countdown;
        state = repeat ? ActiveRepeated : Active;
    }

    bool started() {
        return state > Inactive;
    }

    bool fire() {
        math::real now;
        if (!started() || (now = engine.now()) < alarm)
            return false;
        if (state == ActiveRepeated)
            alarm = now + countdown;
        else
            state = Inactive;
        return true;        
    }
};

} // namespace ge

#endif
