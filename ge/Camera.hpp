#ifndef GE_CAMERA_HPP
#define GE_CAMERA_HPP

#include "glt/Frame.hpp"

#include "ge/Engine.hpp"
#include "ge/Command.hpp"

#include "math/vec2.hpp"

namespace ge {

struct Camera {
    math::vec3_t step_accum;
    float step_length;
    math::vec2_t mouse_sensitivity;
    glt::Frame frame;

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
