#include "ge/Camera.hpp"

#include "math/real.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "util/bit_cast.hpp"
#include "util/string.hpp"

using namespace math;

namespace ge {

struct Handlers
{
    std::shared_ptr<EventHandler<MouseMoved>> mouseMoved;
    std::shared_ptr<EventHandler<InputEvent>> handleInput;
    std::shared_ptr<EventHandler<RenderEvent>> beforeRender;
};

struct Camera::Data
{
    Camera &self;
    glt::Frame frame;
    Commands commands;
    Events events;
    Handlers handlers;
    Engine *engine{};
    std::string frame_path;

    real speed = 0.1_r;
    vec2_t mouse_sensitivity = vec2(0.00075_r);
    vec3_t step_accum = vec3(real(0));
    bool mouse_look{ false };

    explicit Data(Camera & /*me*/);

    void mouseMoved(int16_t dx, int16_t dy);

    // commands
    void runMove(const Event<CommandEvent> & /*unused*/, int64_t dir);
    void runSaveFrame(const Event<CommandEvent> & /*unused*/,
                      ArrayView<const CommandArg> /*args*/);
    void runLoadFrame(const Event<CommandEvent> & /*unused*/,
                      ArrayView<const CommandArg> /*args*/);
    void runSpeed(const Event<CommandEvent> & /*unused*/, double /*s*/);
    void runSensitivity(const Event<CommandEvent> & /*unused*/,
                        ArrayView<const CommandArg> /*args*/);

    // event handlers
    void handleMouseMoved(const Event<MouseMoved> & /*ev*/);
    void handleInput(const Event<InputEvent> & /*unused*/);
    void handleBeforeRender(const Event<RenderEvent> & /*e*/);
};

DECLARE_PIMPL_DEL(Camera)

namespace {

// clang-format off
const std::array<math::real, 36 /* 3 * 12 */> the_dir_table = {
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

Camera::Data::Data(Camera &me) : self(me)
{
    commands.move = makeCommand(
      "camera.move", "move the camera frame", *this, &Data::runMove);

    commands.saveFrame =
      makeCommand("camera.saveFrame",
                  "save the orientation and position of the camera in a file",
                  *this,
                  &Data::runSaveFrame);

    commands.loadFrame =
      makeCommand("camera.loadFrame",
                  "load the orientation and position of the camera from a file",
                  *this,
                  &Data::runLoadFrame);

    commands.speed =
      makeCommand("camera.speed",
                  "set the length the camera is allowed to move in one frame",
                  *this,
                  &Data::runSpeed);

    commands.sensitivity =
      makeCommand("camera.sensitivity",
                  "specify the camera sensitivity: "
                  "either as a pair of numbers (x- and y-sensitvity)"
                  "or as a single number (x- = y-sensitvity)",
                  *this,
                  &Data::runSensitivity);

    handlers.mouseMoved = makeEventHandler(*this, &Data::handleMouseMoved);
    handlers.handleInput = makeEventHandler(*this, &Data::handleInput);
    handlers.beforeRender = makeEventHandler(*this, &Data::handleBeforeRender);
}

void
Camera::Data::runMove(const Event<CommandEvent> & /*unused*/, int64_t dir)
{
    if (dir < 1 || dir > 12) {
        ERR("argument not >= 1 and <= 12");
        return;
    }
    step_accum += vec3(&the_dir_table[3 * (dir - 1)]);
}

void
Camera::Data::runSaveFrame(const Event<CommandEvent> & /*unused*/,
                           ArrayView<const CommandArg> args)
{

    const std::string *path;
    if (args.size() == 0)
        path = &frame_path;
    else if (args.size() == 1 && args[0].type() == CommandArgType::String)
        path = &args[0].string;
    else {
        ERR("invalid parameters: expect 0 or 1 filepath");
        return;
    }

    auto opt_stream = sys::io::HandleStream::open(*path, sys::io::HM_WRITE);
    if (!opt_stream) {
        ERR("couldnt open file: " + *path);
        return;
    }
    auto out_frame = frame;
    out_frame.normalize();
    size_t s = sizeof frame;
    opt_stream->write(s, to_bytes(out_frame).data());
}

void
Camera::Data::runLoadFrame(const Event<CommandEvent> & /*unused*/,
                           ArrayView<const CommandArg> args)
{
    const std::string *path;
    if (args.size() == 0)
        path = &frame_path;
    else if (args.size() == 1 && args[0].type() == CommandArgType::String)
        path = &args[0].string;
    else {
        ERR("invalid parameters: expect 0 or 1 filepath");
        return;
    }

    auto opt_stream = sys::io::HandleStream::open(*path, sys::io::HM_READ);
    if (!opt_stream) {
        WARN("couldnt open file: " + *path);
        return;
    }

    std::array<char, sizeof frame> bytes{};
    size_t s = sizeof bytes;

    auto ret = opt_stream->read(s, bytes.data());
    if (ret != sys::io::StreamResult::OK || s != sizeof frame) {
        ERR(string_concat(
          "failed to read frame from file: ", *path, ", io error: ", ret));
        return;
    }

    memcpy(static_cast<void *>(&frame), bytes.data(), sizeof bytes);
    frame.normalize();
}

void
Camera::Data::runSpeed(const Event<CommandEvent> & /*unused*/, double s)
{
    speed = math::real(s);
}

void
Camera::Data::runSensitivity(const Event<CommandEvent> & /*unused*/,
                             ArrayView<const CommandArg> args)
{
    if (args.size() == 1 && args[0].type() == CommandArgType::Number) {
        mouse_sensitivity = vec2(real(args[0].number));
    } else if (args.size() == 2 && args[0].type() == CommandArgType::Number &&
               args[1].type() == CommandArgType::Number) {
        mouse_sensitivity = vec2(real(args[0].number), real(args[1].number));
    } else {
        ERR("invalid arguments");
    }
}

void
Camera::Data::handleMouseMoved(const Event<MouseMoved> &ev)
{
    self.mouseMoved(ev.info.dx, ev.info.dy);
}

void
Camera::Data::handleInput(const Event<InputEvent> & /*unused*/)
{
    real lenSq = quadrance(step_accum);
    if (lenSq >= math::real(1e-4)) {
        vec3_t local_step = speed * normalize(step_accum);
        Event<CameraMoved> ev =
          Event(CameraMoved(self, transformVector(frame, local_step)));
        if (events.moved.raise(ev))
            frame.translateWorld(ev.info.allowed_step);
    }

    step_accum = vec3(0.f);
}

void
Camera::Data::handleBeforeRender(const Event<RenderEvent> &e)
{
    //    frame->normalize(); TODO: why dont we call that?
    e.info.engine.renderManager().geometryTransform().loadViewMatrix(
      transformationWorldToLocal(frame));
}

void
Camera::Data::mouseMoved(int16_t dx, int16_t dy)
{
    vec2_t rot = vec2(dx, dy) * mouse_sensitivity;
    Event<CameraRotated> re = Event(CameraRotated(self, vec2(-rot[1], rot[0])));
    if (events.rotated.raise(re)) {
        frame.rotateLocal(re.info.allowed_angle[0], vec3(1.f, 0.f, 0.f));
        frame.rotateWorld(re.info.allowed_angle[1], vec3(0.f, 1.f, 0.f));
    }
}

Camera::Camera() : self(new Data(*this)) {}

Camera::~Camera() = default;

Camera::Commands &
Camera::commands()
{
    return self->commands;
}

Camera::Events &
Camera::events()
{
    return self->events;
}

math::vec2_t
Camera::mouseSensitivity() const
{
    return self->mouse_sensitivity;
}

void
Camera::mouseSensitivity(const math::vec2_t &v)
{
    self->mouse_sensitivity = v;
}

real
Camera::speed() const
{
    return self->speed;
}

void
Camera::speed(real s)
{
    self->speed = s;
}

glt::Frame &
Camera::frame()
{
    return self->frame;
}

void
Camera::frame(const glt::Frame &frame)
{
    self->frame = frame;
}

const std::string &
Camera::framePath() const
{
    return self->frame_path;
}

void
Camera::framePath(const std::string &path)
{
    self->frame_path = path;
}

void
Camera::mouseMoved(int16_t dx, int16_t dy)
{
    self->mouseMoved(dx, dy);
}

void
Camera::mouseLook(bool enable)
{
    if (self->mouse_look != enable) {
        self->mouse_look = enable;

        if (!self->engine)
            return;

        if (enable)
            self->engine->window().events().mouseMoved.reg(
              self->handlers.mouseMoved);
        else
            self->engine->window().events().mouseMoved.unreg(
              self->handlers.mouseMoved);
    }
}

void
Camera::registerWith(Engine &e)
{
    self->engine = &e;
    if (self->frame_path.empty())
        self->frame_path = e.programName() + ".cam";
    e.events().beforeRender.reg(self->handlers.beforeRender);
    self->engine->events().handleInput.reg(self->handlers.handleInput);
    if (self->mouse_look) {
        self->mouse_look = false; // force registering of eventhandlers
        mouseLook(true);
    }
}

void
Camera::registerCommands(CommandProcessor &proc)
{
    proc.define(self->commands.move);
    proc.define(self->commands.saveFrame);
    proc.define(self->commands.loadFrame);
    proc.define(self->commands.speed);
    proc.define(self->commands.sensitivity);
}

} // namespace ge
