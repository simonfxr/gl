#include "ge/Camera.hpp"

#include "math/real.hpp"
#include "math/vec3.hpp"

#include <fstream>

using namespace math;

namespace ge {

static Array<vec3_t> mkDirTable();

static const Array<vec3_t> dir_table = mkDirTable();

static void initCommands(Camera& cam);

static void runMove(vec3_t *step_accum, const Event<CommandEvent>&, const Array<CommandArg>& args) {
    int64 dir = args[0].integer;
    if (dir < 1 || dir > 12) {
        ERR("argument not >= 1 and <= 12");
        return;
    }
    *step_accum += dir_table[dir - 1];
}

static void runSaveFrame(const glt::Frame *frame, const Event<CommandEvent>&, const Array<CommandArg>& args) {
    std::ofstream out(args[0].string->c_str());
    if (!out.is_open()) {
        ERR("couldnt open file: " + *args[0].string);
        return;
    }
    
    out.write((const char *) frame, sizeof *frame);
}

static void runLoadFrame(glt::Frame *frame, const Event<CommandEvent>&, const Array<CommandArg>& args) {
    std::ifstream in(args[0].string->c_str());
    if (!in.is_open()) {
        ERR("couldnt open file: " + *args[0].string);
        return;
    }

    in.read((char *) frame, sizeof *frame);
}

static void runSetStepLength(float *step_len, const Event<CommandEvent>&, const Array<CommandArg>& args) {
    *step_len = float(args[0].number);
}

static void runSetSensitivity(vec2_t *sens, const Event<CommandEvent>&, const Array<CommandArg>& args) {
    if (args.size() == 1 && args[0].type == Number) {
        *sens = vec2(args[0].number);
    } else if (args.size() == 2 && args[0].type == Number && args[1].type == Number) {
        *sens = vec2(args[0].number, args[1].number);
    } else {
        ERR("invalid arguments");
    }
}

static void initCommands(Camera& cam) {
    cam.commands.move = makeCommand(runMove, &cam.step_accum,
                                    PARAM_ARRAY(IntegerParam),
                                    "moveCamera", "move the camera frame");
    
    cam.commands.saveFrame = makeCommand(runSaveFrame, static_cast<const glt::Frame *>(&cam.frame),
                                         PARAM_ARRAY(StringParam),
                                         "saveCameraFrame",
                                         "save the orientation and position of the camera in a file");
    
    cam.commands.loadFrame = makeCommand(runLoadFrame, &cam.frame,
                                         PARAM_ARRAY(StringParam),
                                         "loadCameraFrame",
                                         "load the orientation and position of the camera from a file");
    
    cam.commands.setStepLength = makeCommand(runSetStepLength, &cam.step_length,
                                             PARAM_ARRAY(NumberParam),
                                             "setCameraStepLength",
                                             "set the length the camera is allowed to move in one frame");
    
    cam.commands.setSensitivity = makeCommand(runSetSensitivity, &cam.mouse_sensitivity,
                                              PARAM_ARRAY(ListParam),
                                              "setCameraMouseSensitivity",
                                              "specify the camera sensitivity: "
                                              "either as a pair of numbers (x- and y-sensitvity)"
                                              "or as a single number (x- = y-sensitvity)");
}

Camera::Camera(float step_len, vec2_t mouse_sens) :
    step_accum(vec3(0.f)),
    step_length(step_len),
    mouse_sensitivity(mouse_sens)
{
    initCommands(*this);
}

static void mouseLook(Camera *cam, const Event<MouseMoved>& ev) {
//    std::cerr << "mouse look" << std::endl;
    vec2_t rot = vec2(ev.info.dx, ev.info.dy) * cam->mouse_sensitivity;
//    std::cerr << "dx: " << ev.info.dx << " dy: " << ev.info.dy << " rotY: " << rot.y << " rotX: " << rot.x << std::endl;
    cam->frame.rotateLocal(- rot[1], vec3(1.f, 0.f, 0.f));
    cam->frame.rotateWorld(rot[0], vec3(0.f, 1.f, 0.f));
}

static void execStep(Camera *cam, const Event<EngineEvent>&) {
    float lenSq = lengthSq(cam->step_accum);
    
    if (lenSq >= 1e-4f) {
//        std::cerr << "camera step" << std::endl;
        vec3_t local_step = cam->step_length * normalize(cam->step_accum);
        Event<CameraMoved> ev = makeEvent(CameraMoved(transformVector(cam->frame, local_step)));
        cam->moved.raise(ev);
        cam->frame.translateWorld(ev.info.allowed_step);
    }
    
    cam->step_accum = vec3(0.f);
}

static void setCamMat(glt::Frame *frame, const Event<RenderEvent>& e) {
//    frame->normalize();
    e.info.engine.renderManager().geometryTransform().loadViewMatrix(transformationWorldToLocal(*frame));
}

void Camera::registerWith(Engine& e) {
    e.window().events().mouseMoved.reg(makeEventHandler(mouseLook, this));
    e.events().animate.reg(makeEventHandler(execStep, this));
    e.events().beforeRender.reg(makeEventHandler(setCamMat, &frame));
}

void Camera::registerCommands(CommandProcessor& proc) {
    proc.define(commands.move);
    proc.define(commands.saveFrame);
    proc.define(commands.loadFrame);
    proc.define(commands.setStepLength);
    proc.define(commands.setSensitivity);
}

static Array<vec3_t> mkDirTable() {
    Array<vec3_t> dirs(12);

    for (uint32 i = 0; i < 12; ++i)
        dirs[i] = vec3(0.f);

    dirs[11] = vec3(0.f, 0.f, -1.f);
    dirs[5]  = vec3(0.f, 0.f, +1.f);
    dirs[2]  = vec3(+1.f, 0.f, 0.f);
    dirs[8]  = vec3(-1.f, 0.f, 0.f);

    // TODO: add all the clock directions

    return dirs;
}

} // namespace ge
