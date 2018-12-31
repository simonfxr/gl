#ifndef GE_CAMERA_HPP
#define GE_CAMERA_HPP

#include "ge/conf.hpp"

#include "ge/Command.hpp"
#include "ge/Engine.hpp"
#include "ge/Event.hpp"
#include "glt/Frame.hpp"
#include "math/vec2.hpp"

#include <memory>

namespace ge {

struct Camera;

struct GE_API CameraMoved
{
    Camera &camera;
    math::vec3_t step;
    mutable math::vec3_t allowed_step;

    CameraMoved(Camera &cam, const math::vec3_t &s)
      : camera(cam), step(s), allowed_step(s)
    {}
};

struct GE_API CameraRotated
{
    Camera &camera;
    math::vec2_t angle;
    mutable math::vec2_t allowed_angle;

    CameraRotated(Camera &cam, const math::vec2_t &a)
      : camera(cam), angle(a), allowed_angle(a)
    {}
};

struct GE_API Camera : public Plugin
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

    math::vec2_t mouseSensitivity() const;
    void mouseSensitivity(const math::vec2_t &);

    math::real speed() const;
    void speed(math::real);

    glt::Frame &frame();
    void frame(const glt::Frame &);

    const std::string &framePath() const;
    void framePath(const std::string &);

    void mouseMoved(index16 dx, index16 dy);

    void mouseLook(bool enable);

    void registerWith(Engine &e) final override;
    void registerCommands(CommandProcessor &proc) final override;

private:
    struct Data;
    struct DataDeleter
    {
        void operator()(Data *p) noexcept;
    };
    const std::unique_ptr<Data, DataDeleter> self;

    Camera(const Camera &);
    Camera &operator=(const Camera &);
};

} // namespace ge

#endif
