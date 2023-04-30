#include "ge/Camera.hpp"

#include "math/fvec.hpp"
#include "math/real.hpp"
#include "util/string.hpp"

using namespace math;
using namespace bl::literals;

namespace ge {

struct Handlers
{
    std::shared_ptr<EventHandler<MouseMoved>> mouseMoved;
    std::shared_ptr<EventHandler<InputEvent>> handleInput;
    std::shared_ptr<EventHandler<RenderEvent>> beforeRender;
};

struct Camera::Impl : public Camera
{
    glt::Frame frame;
    Commands commands;
    Events events;
    Handlers handlers;
    Engine *engine{};
    bl::string frame_path;

    real speed = 0.1_r;
    vec2 mouse_sensitivity = vec2(0.00075_r);
    vec3 step_accum = vec3(0_r);
    bool mouse_look{ false };

    Impl();
    ~Impl() override;

    void mouseMoved(int16_t dx, int16_t dy);

    // commands
    void runMove(const Event<CommandEvent> & /*unused*/, int64_t dir);
    void runSaveFrame(const Event<CommandEvent> & /*unused*/,
                      bl::span<const CommandArg> /*args*/);
    void runLoadFrame(const Event<CommandEvent> & /*unused*/,
                      bl::span<const CommandArg> /*args*/);
    void runSpeed(const Event<CommandEvent> & /*unused*/, double /*s*/);
    void runSensitivity(const Event<CommandEvent> & /*unused*/,
                        bl::span<const CommandArg> /*args*/);

    // event handlers
    void handleMouseMoved(const Event<MouseMoved> & /*ev*/);
    void handleInput(const Event<InputEvent> & /*unused*/);
    void handleBeforeRender(const Event<RenderEvent> & /*e*/) const;
};

BL_DEFINE_RUNTIME_SIZED(Camera, Camera::Impl);

Camera::Impl::~Impl() = default;

namespace {

// clang-format off
const bl::array<math::real, 36 /* 3 * 12 */> the_dir_table = {
    0.5f, 0.f, -0.8660254037844386f,
    0.8660254037844387f, 0.f, -0.5f,
    1.f, 0.f, 0.f,
    0.8660254037844384f, 0.f, 0.5f,
    0.5f, 0.f, 0.8660254037844386f,
    0.f, 0.f, 1.f,
    -0.5f, 0.f, 0.8660254037844384f,
    -0.8660254037844386f, 0.f, 0.5f,
    -1.f, 0.f, 0.f,
    -0.8660254037844387f, 0.f, -0.5f,
    -0.5f, 0.f, -0.8660254037844387f,
    0.f, 0.f, -1.f
};
// clang-format on

} // namespace

Camera::Impl::Impl()
{
    frame.origin = math::vec3(0.f);

    commands.move = makeCommand(
      "camera.move"_k, "move the camera frame"_k, *this, &Impl::runMove);

    commands.saveFrame =
      makeCommand("camera.saveFrame"_k,
                  "save the orientation and position of the camera in a file"_k,
                  *this,
                  &Impl::runSaveFrame);

    commands.loadFrame = makeCommand(
      "camera.loadFrame"_k,
      "load the orientation and position of the camera from a file"_k,
      *this,
      &Impl::runLoadFrame);

    commands.speed =
      makeCommand("camera.speed"_k,
                  "set the length the camera is allowed to move in one frame"_k,
                  *this,
                  &Impl::runSpeed);

    commands.sensitivity =
      makeCommand("camera.sensitivity"_k,
                  bl::string_view_literal(
                    "specify the camera sensitivity: "
                    "either as a pair of numbers (x- and y-sensitvity)"
                    "or as a single number (x- = y-sensitvity)"),
                  *this,
                  &Impl::runSensitivity);

    handlers.mouseMoved = makeEventHandler(*this, &Impl::handleMouseMoved);
    handlers.handleInput = makeEventHandler(*this, &Impl::handleInput);
    handlers.beforeRender = makeEventHandler(*this, &Impl::handleBeforeRender);
}

void
Camera::Impl::runMove(const Event<CommandEvent> & /*unused*/, int64_t dir)
{
    if (dir < 1 || dir > 12) {
        ERR("argument not >= 1 and <= 12");
        return;
    }
    auto p = &the_dir_table[3 * (dir - 1)];
    step_accum += vec3(p[0], p[1], p[2]);
}

void
Camera::Impl::runSaveFrame(const Event<CommandEvent> & /*unused*/,
                           bl::span<const CommandArg> args)
{

    const bl::string *path;
    if (args.empty())
        path = &frame_path;
    else if (args.size() == 1 && args[0].type() == CommandArgType::String)
        path = &args[0].string;
    else {
        ERR("invalid parameters: expect 0 or 1 filepath");
        return;
    }

    auto res = sys::io::HandleStream::open(*path, sys::io::HM_WRITE);
    if (!res) {
        ERR(scat("couldnt open file: "_sv, *path));
        return;
    }
    auto out_frame = frame;
    out_frame.normalize();
    sys::io::write(res.value(), bl::as_bytes(bl::span{ &out_frame, 1 }));
}

void
Camera::Impl::runLoadFrame(const Event<CommandEvent> & /*unused*/,
                           bl::span<const CommandArg> args)
{
    const bl::string *path;
    if (args.empty())
        path = &frame_path;
    else if (args.size() == 1 && args[0].type() == CommandArgType::String)
        path = &args[0].string;
    else {
        ERR("invalid parameters: expect 0 or 1 filepath");
        return;
    }

    auto res = sys::io::HandleStream::open(*path, sys::io::HM_READ);
    if (!res) {
        WARN(scat("couldnt open file: "_sv, *path));
        return;
    }

    bl::array<bl::byte, sizeof frame> bytes{};
    auto [s, ret] = sys::io::read(res.value(), bytes);
    if (ret != sys::io::StreamResult::OK || s != sizeof frame) {
        ERR(scat("failed to read frame from file: "_sv,
                 *path,
                 ", io error: "_sv,
                 ret));
        return;
    }

    frame = std::bit_cast<glt::Frame>(bytes);
    frame.normalize();
}

void
Camera::Impl::runSpeed(const Event<CommandEvent> & /*unused*/, double s)
{
    speed = static_cast<math::real>(s);
}

void
Camera::Impl::runSensitivity(const Event<CommandEvent> & /*unused*/,
                             bl::span<const CommandArg> args)
{
    if (args.size() == 1 && args[0].type() == CommandArgType::Number) {
        mouse_sensitivity = vec2(static_cast<real>(args[0].number));
    } else if (args.size() == 2 && args[0].type() == CommandArgType::Number &&
               args[1].type() == CommandArgType::Number) {
        mouse_sensitivity = vec2(static_cast<real>(args[0].number),
                                 static_cast<real>(args[1].number));
    } else {
        ERR("invalid arguments");
    }
}

void
Camera::Impl::handleMouseMoved(const Event<MouseMoved> &ev)
{
    m().mouseMoved(ev.info.dx, ev.info.dy);
}

void
Camera::Impl::handleInput(const Event<InputEvent> & /*unused*/)
{
    auto lenSq = quadrance(step_accum);
    if (lenSq >= 1e-4_r) {
        auto local_step = speed * normalize(step_accum);
        auto ev = Event(CameraMoved(*this, transformVector(frame, local_step)));
        if (events.moved.raise(ev))
            frame.translateWorld(ev.info.allowed_step);
    }

    step_accum = vec3(0.f);
}

void
Camera::Impl::handleBeforeRender(const Event<RenderEvent> &e) const
{
    //    frame->normalize(); TODO: why dont we call that?
    e.info.engine.renderManager().geometryTransform().loadViewMatrix(
      transformationWorldToLocal(frame));
}

void
Camera::Impl::mouseMoved(int16_t dx, int16_t dy)
{
    vec2 rot = vec2(dx, dy) * mouse_sensitivity;
    Event<CameraRotated> re =
      Event(CameraRotated(*this, vec2(-rot[1], rot[0])));
    if (events.rotated.raise(re)) {
        frame.rotateLocal(re.info.allowed_angle[0], vec3(1.f, 0.f, 0.f));
        frame.rotateWorld(re.info.allowed_angle[1], vec3(0.f, 1.f, 0.f));
    }
}

Camera &
Camera::construct_at(void *p) noexcept
{
    return construct_impl(p);
}

Camera::Commands &
Camera::commands()
{
    return m().commands;
}

Camera::Events &
Camera::events()
{
    return m().events;
}

math::vec2
Camera::mouseSensitivity() const
{
    return m().mouse_sensitivity;
}

void
Camera::mouseSensitivity(const math::vec2 &v)
{
    m().mouse_sensitivity = v;
}

real
Camera::speed() const
{
    return m().speed;
}

void
Camera::speed(real s)
{
    m().speed = s;
}

glt::Frame &
Camera::frame()
{
    return m().frame;
}

void
Camera::frame(const glt::Frame &frame)
{
    m().frame = frame;
}

const bl::string &
Camera::framePath() const
{
    return m().frame_path;
}

void
Camera::framePath(const bl::string &path)
{
    m().frame_path = path;
}

void
Camera::mouseMoved(int16_t dx, int16_t dy)
{
    m().mouseMoved(dx, dy);
}

void
Camera::mouseLook(bool enable)
{
    if (m().mouse_look != enable) {
        m().mouse_look = enable;

        if (!m().engine)
            return;

        if (enable)
            m().engine->window().events().mouseMoved.reg(
              m().handlers.mouseMoved);
        else
            m().engine->window().events().mouseMoved.unreg(
              m().handlers.mouseMoved);
    }
}

void
Camera::registerWith(Engine &e)
{
    m().engine = &e;
    if (m().frame_path.empty())
        m().frame_path = scat(e.programName(), ".cam"_sv);
    e.events().beforeRender.reg(m().handlers.beforeRender);
    m().engine->events().handleInput.reg(m().handlers.handleInput);
    if (m().mouse_look) {
        m().mouse_look = false; // force registering of eventhandlers
        mouseLook(true);
    }
}

void
Camera::registerCommands(CommandProcessor &proc)
{
    proc.define(m().commands.move);
    proc.define(m().commands.saveFrame);
    proc.define(m().commands.loadFrame);
    proc.define(m().commands.speed);
    proc.define(m().commands.sensitivity);
}

} // namespace ge
