#include "math/real.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"

#include "ge/Camera.hpp"
#include "ge/CommandParams.hpp"

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
    real speed = 0.1_r;
    vec2_t mouse_sensitivity = vec2(0.00075_r);
    glt::Frame frame;
    Commands commands;
    Events events;
    vec3_t step_accum = vec3(real(0));
    Handlers handlers;
    bool mouse_look{ false };
    Engine *engine{};
    std::string frame_path;

    explicit Data(Camera & /*me*/);

    void mouseMoved(int16_t dx, int16_t dy);

    // commands
    void runMove(const Event<CommandEvent> & /*unused*/,
                 const Array<CommandArg> & /*args*/);
    void runSaveFrame(const Event<CommandEvent> & /*unused*/,
                      const Array<CommandArg> & /*args*/);
    void runLoadFrame(const Event<CommandEvent> & /*unused*/,
                      const Array<CommandArg> & /*args*/);
    void runSpeed(const Event<CommandEvent> & /*unused*/,
                  const Array<CommandArg> & /*args*/);
    void runSensitivity(const Event<CommandEvent> & /*unused*/,
                        const Array<CommandArg> & /*args*/);

    // event handlers
    static void handleMouseMoved(Camera * /*cam*/,
                                 const Event<MouseMoved> & /*ev*/);
    static void handleInput(Camera * /*cam*/,
                            const Event<InputEvent> & /*unused*/);
    void handleBeforeRender(const Event<RenderEvent> & /*e*/);
};

DECLARE_PIMPL_DEL(Camera)

namespace {

const math::real the_dir_table[3 * 12] = { 0.5f,
                                           0.f,
                                           -0.8660254037844386f,
                                           0.8660254037844387f,
                                           0.f,
                                           -0.5f,
                                           1.f,
                                           0.f,
                                           0.f,
                                           0.8660254037844384f,
                                           0.f,
                                           0.5f,
                                           0.5f,
                                           0.f,
                                           0.8660254037844386f,
                                           0.f,
                                           0.f,
                                           1.f,
                                           -0.5f,
                                           0.f,
                                           0.8660254037844384f,
                                           -0.8660254037844386f,
                                           0.f,
                                           0.5f,
                                           -1.f,
                                           0.f,
                                           0.f,
                                           -0.8660254037844387f,
                                           0.f,
                                           -0.5f,
                                           -0.5f,
                                           0.f,
                                           -0.8660254037844387f,
                                           0.f,
                                           0.f,
                                           -1.f };

} // namespace

Camera::Data::Data(Camera &me) : self(me)
{
    commands.move = makeCommand(
      this, &Data::runMove, INT_PARAMS, "camera.move", "move the camera frame");

    commands.saveFrame =
      makeCommand(this,
                  &Data::runSaveFrame,
                  LIST_PARAMS,
                  "camera.saveFrame",
                  "save the orientation and position of the camera in a file");

    commands.loadFrame = makeCommand(
      this,
      &Data::runLoadFrame,
      LIST_PARAMS,
      "camera.loadFrame",
      "load the orientation and position of the camera from a file");

    commands.speed =
      makeCommand(this,
                  &Data::runSpeed,
                  NUM_PARAMS,
                  "camera.speed",
                  "set the length the camera is allowed to move in one frame");

    commands.sensitivity =
      makeCommand(this,
                  &Data::runSensitivity,
                  LIST_PARAMS,
                  "camera.sensitivity",
                  "specify the camera sensitivity: "
                  "either as a pair of numbers (x- and y-sensitvity)"
                  "or as a single number (x- = y-sensitvity)");

    handlers.mouseMoved = makeEventHandler(&Data::handleMouseMoved, &me);
    handlers.handleInput = makeEventHandler(&Data::handleInput, &me);
    handlers.beforeRender = makeEventHandler(this, &Data::handleBeforeRender);
}

void
Camera::Data::runMove(const Event<CommandEvent> & /*unused*/,
                      const Array<CommandArg> &args)
{
    auto dir = args[0].integer;
    if (dir < 1 || dir > 12) {
        ERR("argument not >= 1 and <= 12");
        return;
    }
    step_accum += vec3(&the_dir_table[3 * (dir - 1)]);
}

void
Camera::Data::runSaveFrame(const Event<CommandEvent> & /*unused*/,
                           const Array<CommandArg> &args)
{

    const std::string *path;
    if (args.size() == 0)
        path = &frame_path;
    else if (args.size() == 1 && args[0].type == String)
        path = args[0].string;
    else {
        ERR("invalid parameters: expect 0 or 1 filepath");
        return;
    }

    auto opt_stream = sys::io::HandleStream::open(*path, sys::io::HM_WRITE);
    if (!opt_stream) {
        ERR("couldnt open file: " + *path);
        return;
    }
    size_t s = sizeof frame;
    opt_stream->write(s, reinterpret_cast<const char *>(&frame));
}

void
Camera::Data::runLoadFrame(const Event<CommandEvent> & /*unused*/,
                           const Array<CommandArg> &args)
{

    const std::string *path;
    if (args.size() == 0)
        path = &frame_path;
    else if (args.size() == 1 && args[0].type == String)
        path = args[0].string;
    else {
        ERR("invalid parameters: expect 0 or 1 filepath");
        return;
    }

    auto opt_stream = sys::io::HandleStream::open(*path, sys::io::HM_READ);
    if (!opt_stream) {
        ERR("couldnt open file: " + *path);
        return;
    }

    size_t s = sizeof frame;
    opt_stream->read(s, reinterpret_cast<char *>(&frame));
}

void
Camera::Data::runSpeed(const Event<CommandEvent> & /*unused*/,
                       const Array<CommandArg> &args)
{
    speed = math::real(args[0].number);
}

void
Camera::Data::runSensitivity(const Event<CommandEvent> & /*unused*/,
                             const Array<CommandArg> &args)
{
    if (args.size() == 1 && args[0].type == Number) {
        mouse_sensitivity = vec2(real(args[0].number));
    } else if (args.size() == 2 && args[0].type == Number &&
               args[1].type == Number) {
        mouse_sensitivity = vec2(real(args[0].number), real(args[1].number));
    } else {
        ERR("invalid arguments");
    }
}

void
Camera::Data::handleMouseMoved(Camera *cam, const Event<MouseMoved> &ev)
{
    cam->mouseMoved(ev.info.dx, ev.info.dy);
}

void
Camera::Data::handleInput(Camera *cam, const Event<InputEvent> & /*unused*/)
{
    auto &self = *cam->self;
    real lenSq = quadrance(self.step_accum);
    if (lenSq >= math::real(1e-4)) {
        vec3_t local_step = self.speed * normalize(self.step_accum);
        Event<CameraMoved> ev =
          makeEvent(CameraMoved(*cam, transformVector(self.frame, local_step)));
        if (self.events.moved.raise(ev))
            self.frame.translateWorld(ev.info.allowed_step);
    }

    self.step_accum = vec3(0.f);
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
    Event<CameraRotated> re =
      makeEvent(CameraRotated(self, vec2(-rot[1], rot[0])));
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
