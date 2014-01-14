#include <fstream>

#include "math/real.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"

#include "ge/Camera.hpp"
#include "ge/CommandParams.hpp"

using namespace math;

namespace ge {

struct Handlers {
    Ref<EventHandler<MouseMoved> > mouseMoved;
    Ref<EventHandler<InputEvent> > handleInput;
    Ref<EventHandler<RenderEvent> > beforeRender;
};

struct Camera::Data {

    real _speed;
    vec2_t _mouse_sensitivity;
    glt::Frame _frame;
    Commands _commands;
    Events _events;
    vec3_t _step_accum;
    Handlers _handlers;
    bool _mouse_look;
    Engine *_engine;
    std::string _frame_path;

    Data(Camera *);

    void mouseMoved(Camera *cam, index16 dx, index16 dy);

    // commands
    void runMove(const Event<CommandEvent>&, const Array<CommandArg>&);
    void runSaveFrame(const Event<CommandEvent>&, const Array<CommandArg>&);
    void runLoadFrame(const Event<CommandEvent>&, const Array<CommandArg>&);
    void runSpeed(const Event<CommandEvent>&, const Array<CommandArg>&);
    void runSensitivity(const Event<CommandEvent>&, const Array<CommandArg>&);

    // event handlers
    static void handleMouseMoved(Camera *, const Event<MouseMoved>&);
    static void handleInput(Camera *, const Event<InputEvent>&);
    void handleBeforeRender(const Event<RenderEvent>&);
};

namespace {

const math::real the_dir_table[3 * 12] = {
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

} // namespace anon

Camera::Data::Data(Camera *me) :
    _speed(real(0.1)),
    _mouse_sensitivity(vec2(real(0.00075))),
    _frame(),
    _commands(),
    _events(),
    _step_accum(vec3(real(0))),
    _handlers(),
    _mouse_look(false),
    _engine(0),
    _frame_path()
{
    _commands.move = makeCommand(this, &Data::runMove,
                                 INT_PARAMS,
                                 "camera.move", "move the camera frame");
    
    _commands.saveFrame = makeCommand(this, &Data::runSaveFrame,
                                      LIST_PARAMS,
                                      "camera.saveFrame",
                                      "save the orientation and position of the camera in a file");
    
    _commands.loadFrame = makeCommand(this, &Data::runLoadFrame,
                                      LIST_PARAMS,
                                      "camera.loadFrame",
                                      "load the orientation and position of the camera from a file");
    
    _commands.speed = makeCommand(this, &Data::runSpeed,
                                  NUM_PARAMS,
                                  "camera.speed",
                                  "set the length the camera is allowed to move in one frame");
    
    _commands.sensitivity = makeCommand(this, &Data::runSensitivity,
                                        LIST_PARAMS,
                                        "camera.sensitivity",
                                        "specify the camera sensitivity: "
                                        "either as a pair of numbers (x- and y-sensitvity)"
                                        "or as a single number (x- = y-sensitvity)");

    _handlers.mouseMoved = makeEventHandler(&Data::handleMouseMoved, me);
    _handlers.handleInput = makeEventHandler(&Data::handleInput, me);
    _handlers.beforeRender = makeEventHandler(this, &Data::handleBeforeRender);
}

void Camera::Data::runMove(const Event<CommandEvent>&, const Array<CommandArg>& args) {
    int64 dir = args[0].integer;
    if (dir < 1 || dir > 12) {
        ERR("argument not >= 1 and <= 12");
        return;
    }
    _step_accum += vec3(&the_dir_table[3 * (dir - 1)]);
}

void Camera::Data::runSaveFrame(const Event<CommandEvent>&, const Array<CommandArg>& args) {

    const std::string *path;
    if (args.size() == 0)
        path = &_frame_path;
    else if (args.size() == 1 && args[0].type == String)
        path = args[0].string;
    else {
        ERR("invalid parameters: expect 0 or 1 filepath");
        return;
    }
    
    std::ofstream out(path->c_str());
    if (!out.is_open()) {
        ERR("couldnt open file: " + *path);
        return;
    }
    
    out.write(reinterpret_cast<const char *>(&_frame), sizeof _frame);
}

void Camera::Data::runLoadFrame(const Event<CommandEvent>&, const Array<CommandArg>& args) {

    const std::string *path;
    if (args.size() == 0)
        path = &_frame_path;
    else if (args.size() == 1 && args[0].type == String)
        path = args[0].string;
    else {
        ERR("invalid parameters: expect 0 or 1 filepath");
        return;
    }

    std::ifstream in(path->c_str());
    if (!in.is_open()) {
        ERR("couldnt open file: " + *path);
        return;
    }

    in.read(reinterpret_cast<char *>(&_frame), sizeof _frame);
}

void Camera::Data::runSpeed(const Event<CommandEvent>&, const Array<CommandArg>& args) {
    _speed = float(args[0].number);
}

void Camera::Data::runSensitivity(const Event<CommandEvent>&, const Array<CommandArg>& args) {
    if (args.size() == 1 && args[0].type == Number) {
        _mouse_sensitivity = vec2(real(args[0].number));
    } else if (args.size() == 2 && args[0].type == Number && args[1].type == Number) {
        _mouse_sensitivity = vec2(real(args[0].number), real(args[1].number));
    } else {
        ERR("invalid arguments");
    }
}

void Camera::Data::handleMouseMoved(Camera *cam, const Event<MouseMoved>& ev) {
    cam->mouseMoved(ev.info.dx, ev.info.dy);
}

void Camera::Data::handleInput(Camera *cam, const Event<InputEvent>&) {
    Data *self = cam->self;
    real lenSq = lengthSq(self->_step_accum);
    if (lenSq >= 1e-4f) {
        vec3_t local_step = self->_speed * normalize(self->_step_accum);
        Event<CameraMoved> ev = makeEvent(CameraMoved(*cam, transformVector(self->_frame, local_step)));
        if (self->_events.moved.raise(ev))
            self->_frame.translateWorld(ev.info.allowed_step);
    }
    
    self->_step_accum = vec3(0.f);
}

void Camera::Data::handleBeforeRender(const Event<RenderEvent>& e) {
//    frame->normalize(); TODO: why dont we call that?
    e.info.engine.renderManager().geometryTransform().loadViewMatrix(transformationWorldToLocal(_frame));
}

void Camera::Data::mouseMoved(Camera *cam, index16 dx, index16 dy) {
    Data *self = cam->self;
    vec2_t rot = vec2(dx, dy) * _mouse_sensitivity;
    Event<CameraRotated> re = makeEvent(CameraRotated(*cam, vec2(-rot[1], rot[0])));
    if (self->_events.rotated.raise(re)) {
        self->_frame.rotateLocal(re.info.allowed_angle[0], vec3(1.f, 0.f, 0.f));
        self->_frame.rotateWorld(re.info.allowed_angle[1], vec3(0.f, 1.f, 0.f));
    }
}

Camera::Camera() :
    self(new Data(this))
{}

Camera::~Camera() {
    delete self;
}

Camera::Commands& Camera::commands() {
    return self->_commands;
}

Camera::Events& Camera::events() {
    return self->_events;
}

math::vec2_t Camera::mouseSensitivity() const {
    return self->_mouse_sensitivity;
}

void  Camera::mouseSensitivity(const math::vec2_t& v) {
    self->_mouse_sensitivity = v;
}

real Camera::speed() const {
    return self->_speed;
}

void Camera::speed(real s) {
    self->_speed = s;
}

glt::Frame& Camera::frame() {
    return self->_frame;
}

void Camera::frame(const glt::Frame& frame) {
    self->_frame = frame;
}

const std::string& Camera::framePath() const {
    return self->_frame_path;
}

void Camera::framePath(const std::string& path) {
    self->_frame_path = path;
}

void Camera::mouseMoved(index16 dx, index16 dy) {
    self->mouseMoved(this, dx, dy);
}

void Camera::mouseLook(bool enable) {
    if (self->_mouse_look != enable) {
        self->_mouse_look = enable;

        if (!self->_engine)
            return;
        
        if (enable)
            self->_engine->window().events().mouseMoved.reg(self->_handlers.mouseMoved);
        else
            self->_engine->window().events().mouseMoved.unreg(self->_handlers.mouseMoved);
    }
}

void Camera::registerWith(Engine& e) {
    self->_engine = &e;
    if (self->_frame_path.empty())
        self->_frame_path = e.programName() + ".cam";
    e.events().beforeRender.reg(self->_handlers.beforeRender);
    self->_engine->events().handleInput.reg(self->_handlers.handleInput);
    if (self->_mouse_look) {
        self->_mouse_look = false; // force registering of eventhandlers
        mouseLook(true);
    }
}

void Camera::registerCommands(CommandProcessor& proc) {
    proc.define(self->_commands.move);
    proc.define(self->_commands.saveFrame);
    proc.define(self->_commands.loadFrame);
    proc.define(self->_commands.speed);
    proc.define(self->_commands.sensitivity);
}

} // namespace ge
