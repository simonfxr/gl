#include "ge/Camera.hpp"

#include "math/math.hpp"
#include "math/vec3.hpp"

using namespace math;

namespace ge {

static Array<vec3_t> mkDirTable();

static const Array<vec3_t> dir_table = mkDirTable();

struct MoveCommand : public Command {
    vec3_t& step_accum;
    
    MoveCommand(vec3_t& acc) :
        Command(CONST_ARRAY(CommandParamType, IntegerParam),
                "",
                "move the camera frame"),
        step_accum(acc) {}

    void interactive(const Event<CommandEvent>&, const Array<CommandArg>& args) {
        int64 dir = args[0].integer;
        if (dir < 1 || dir > 12) {
            ERR("argument not >= 1 and <= 12");
            return;
        }
        step_accum += dir_table[dir - 1];
    }
};

Camera::Camera(float step_len, vec2_t mouse_sens) :
    step_accum(vec3(0.f)),
    step_length(step_len),
    mouse_sensitivity(mouse_sens),
    moveCommand(new MoveCommand(step_accum))
{}

static void mouseLook(Camera *cam, const Event<MouseMoved>& ev) {
//    std::cerr << "mouse look" << std::endl;
    vec2_t rot = vec2(ev.info.dx, ev.info.dy) * cam->mouse_sensitivity;
//    std::cerr << "dx: " << ev.info.dx << " dy: " << ev.info.dy << " rotY: " << rot.y << " rotX: " << rot.x << std::endl;
    cam->frame.rotateLocal(-rot.y, vec3(1.f, 0.f, 0.f));
    cam->frame.rotateWorld( rot.x, vec3(0.f, 1.f, 0.f));
}

static void execStep(Camera *cam, const Event<EngineEvent>&) {
    float lenSq = lengthSq(cam->step_accum);
    if (!lenSq < 1e-4f) {
//        std::cerr << "camera step" << std::endl;
        cam->frame.translateLocal(cam->step_length * normalize(cam->step_accum));
    }
    cam->step_accum = vec3(0.f);
}

void Camera::registerWith(Engine& e, const std::string& move_cmd) {
    if (!move_cmd.empty())
        e.commandProcessor().define(move_cmd, moveCommand);
    e.window().events().mouseMoved.reg(makeEventHandler(mouseLook, this));
    e.events().animate.reg(makeEventHandler(execStep, this));
}

static Array<vec3_t> mkDirTable() {
    Array<vec3_t> dirs(12);

    for (uint32 i = 0; i < 12; ++i)
        dirs[i] = vec3(0.f);

    dirs[11] = vec3(0.f, 0.f, +1.f);
    dirs[5]  = vec3(0.f, 0.f, -1.f);
    dirs[2]  = vec3(+1.f, 0.f, 0.f);
    dirs[8]  = vec3(-1.f, 0.f, 0.f);

    // TODO: add all the clock directions

    return dirs;
}

} // namespace ge
