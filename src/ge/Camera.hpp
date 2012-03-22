#ifndef GE_CAMERA_HPP
#define GE_CAMERA_HPP

#include "glt/Frame.hpp"

#include "ge/conf.hpp"
#include "ge/Engine.hpp"
#include "ge/Command.hpp"
#include "ge/Event.hpp"

#include "math/vec2.hpp"

namespace ge {

struct Camera;

struct GE_API CameraMoved {
    Camera& camera;
    math::vec3_t step;
    mutable math::vec3_t allowed_step;

    CameraMoved(Camera& cam, const math::vec3_t& s) :
        camera(cam), step(s), allowed_step(s) {}
};

struct GE_API CameraRotated {
    Camera& camera;
    math::vec2_t angle;
    mutable math::vec2_t allowed_angle;
    
    CameraRotated(Camera& cam, const math::vec2_t& a) :
        camera(cam), angle(a), allowed_angle(a) {}
};

struct GE_API Camera {
    math::vec3_t step_accum;
    float step_length;
    math::vec2_t mouse_sensitivity;
    glt::Frame frame;
    
    EventSource<CameraMoved> moved;
    EventSource<CameraRotated> rotated;

    struct Commands {
        Ref<Command> move;
        Ref<Command> saveFrame;
        Ref<Command> loadFrame;
        Ref<Command> setStepLength;
        Ref<Command> setSensitivity;
    };

    Commands commands;
    
    Camera(float step_len = 0.1f, math::vec2_t mouse_sens = math::vec2(0.0005f, 0.0005f));
    
    void registerWith(Engine& e);
    void registerCommands(CommandProcessor& proc);
};

} // namespace ge

#endif
