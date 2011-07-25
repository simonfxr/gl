#ifndef GE_CAMERA_HPP
#define GE_CAMERA_HPP

#include "glt/Frame.hpp"

#include "ge/Engine.hpp"
#include "ge/Command.hpp"
#include "ge/Event.hpp"

#include "math/vec2.hpp"

namespace ge {

struct CameraMoved {
    math::vec3_t step;
    mutable math::vec3_t allowed_step;

    CameraMoved(const math::vec3_t& s) :
        step(s), allowed_step(s) {}
};

struct Camera {
    math::vec3_t step_accum;
    float step_length;
    math::vec2_t mouse_sensitivity;
    glt::Frame frame;
    EventSource<CameraMoved> moved;

    struct Commands {
        Ref<Command> move;
        Ref<Command> saveFrame;
        Ref<Command> loadFrame;
        Ref<Command> setStepLength;
        Ref<Command> setSensitivity;
    };

    Commands commands;
    
    Camera(float step_len = 0.1f, vec2_t mouse_sens = vec2(0.0005f, 0.0005f));
    
    void registerWith(Engine& e);
    void registerCommands(CommandProcessor& proc);
};

} // namespace ge

#endif
