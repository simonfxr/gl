#ifndef GE_CAMERA_HPP
#define GE_CAMERA_HPP

#include "ge/conf.hpp"

#include "ge/Command.hpp"
#include "ge/Engine.hpp"
#include "ge/Event.hpp"
#include "glt/Frame.hpp"
#include "math/fvec.hpp"

#include <memory>

namespace ge {

struct Camera;

struct GE_API CameraMoved
{
    Camera &camera;
    math::vec3 step;
    mutable math::vec3 allowed_step;

    CameraMoved(Camera &cam, const math::vec3 &s)
      : camera(cam), step(s), allowed_step(s)
    {}
};

struct GE_API CameraRotated
{
    Camera &camera;
    math::vec2 angle;
    mutable math::vec2 allowed_angle;

    CameraRotated(Camera &cam, const math::vec2 &a)
      : camera(cam), angle(a), allowed_angle(a)
    {}
};

struct GE_API Camera final : public PluginImpl<Camera>
{
    struct Events
    {
        EventSource<CameraMoved> moved;
        EventSource<CameraRotated> rotated;
    };

    struct Commands
    {
        CommandPtr move;
        CommandPtr saveFrame;
        CommandPtr loadFrame;
        CommandPtr speed;
        CommandPtr sensitivity;
    };

    Camera();
    ~Camera() override;

    Commands &commands();

    Events &events();

    math::vec2 mouseSensitivity() const;
    void mouseSensitivity(const math::vec2 &);

    math::real speed() const;
    void speed(math::real);

    glt::Frame &frame();
    void frame(const glt::Frame &);

    const std::string &framePath() const;
    void framePath(const std::string &);

    void mouseMoved(int16_t dx, int16_t dy);

    void mouseLook(bool enable);

    void registerWith(Engine &e) final override;
    void registerCommands(CommandProcessor &proc) final override;

private:
    DECLARE_PIMPL(GE_API, self);
};

} // namespace ge

#endif
