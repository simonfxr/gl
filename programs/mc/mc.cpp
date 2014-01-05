#include "ge/Engine.hpp"

#include "math/real.hpp"
#include "math/vec2.hpp"

#include "glt/ShaderProgram.hpp"
#include "glt/Mesh.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/TextureRenderTarget.hpp"
#include "glt/utils.hpp"

#include "shaders/shader_constants.h"

#include "ge/Engine.hpp"

#include <vector>
#include <string>
#include <sstream>

#include <CL/cl.h>
#include <CL/cl.hpp>

#ifdef SYSTEM_LINUX
#include <GL/glx.h>
#endif

using namespace math;

struct Vertex {
    vec2_t position;
};

DEFINE_VERTEX_DESC(Vertex,
                   VERTEX_ATTR(Vertex, position));

struct Anim {
    ge::Engine engine;
    real time_print_fps;
    
    cl::Context cl_ctx;
    cl::CommandQueue cl_q;
    
    void init(const ge::Event<ge::InitEvent>&);
    void initCL(const ge::Event<ge::InitEvent>&);
    void link(ge::Engine& e);
    void animate(const ge::Event<ge::AnimationEvent>&);
    void renderScene(const ge::Event<ge::RenderEvent>&);
    
    void handleWindowResized(const ge::Event<ge::WindowResized>& ev);
};

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    link(ev.info.engine);
    time_print_fps = 0;

    ev.info.success = true;
}

static void CL_CALLBACK print_cl_error(const char *msg, const void *, size_t, void *) {
    sys::io::stdout() << msg << sys::io::endl;
}

static cl::Context createCLGLContext(cl::Platform& platform, cl_int *err) {

    cl_context_properties props[] = {
#ifdef SYSTEM_WINDOWS
        CL_GL_CONTEXT_KHR,
        (cl_context_properties) wglGetCurrentContext(),
        CL_WGL_HDC_KHR,
        (cl_context_properties) wglGetCurrentDC(),
        CL_CONTEXT_PLATFORM,
        (cl_context_properties) (platform)(),
#elif defined(SYSTEM_LINUX)
        CL_GL_CONTEXT_KHR,
        (cl_context_properties) glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR,
        (cl_context_properties) glXGetCurrentDisplay(),
        CL_CONTEXT_PLATFORM,
        (cl_context_properties) (platform)(),
#else
#error "unknown system"
#endif        
        0
    };

    return cl::Context(CL_DEVICE_TYPE_GPU, props, print_cl_error, NULL, err);
}

void Anim::initCL(const ge::Event<ge::InitEvent>& ev) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    engine.out() << "found " << platforms.size() << " CL platforms" << sys::io::endl;
    if (platforms.size() < 1)
        return;
    platforms.resize(1);

    cl::Platform& platform = platforms[0];
    std::string info_value;
    engine.out() << "selecting CL platform:" << sys::io::endl;
    
#define PLAT_INFO(info)                                                \
    platform.getInfo(info, &info_value);                                \
    engine.out() << "  " << #info << ": " << info_value << sys::io::endl

    PLAT_INFO(CL_PLATFORM_NAME);
    PLAT_INFO(CL_PLATFORM_PROFILE);
    PLAT_INFO(CL_PLATFORM_VENDOR);
    PLAT_INFO(CL_PLATFORM_VERSION);
#undef PLAT_INFO
    
    engine.out() << "  CL_PLATFORM_EXTENSIONS:" << sys::io::endl;
    platform.getInfo(CL_PLATFORM_EXTENSIONS, &info_value);
    std::istringstream exts(info_value);
    std::string ext;
    while(std::getline(exts, ext, ' '))
        engine.out() << "    " << ext << sys::io::endl;

    engine.out() << "  END EXTENSIONS" << sys::io::endl;

    cl_int err;
    cl_ctx = createCLGLContext(platform, &err);
    if (err != CL_SUCCESS) {
        ERR("createing shared OpenCL/OpenGL context failed");
        return;
    }

    engine.out() << "created CL context" << sys::io::endl;
    
    std::vector <cl::Device> devices = cl_ctx.getInfo<CL_CONTEXT_DEVICES>();
    if (devices.size() < 1)
        return;
    devices.resize(1);
    engine.out() << "selecting CL device:" << sys::io::endl;
    cl::Device& dev = devices[0];

#define DEV_INFO(info)                                                  \
    dev.getInfo(info, &info_value);                                     \
    engine.out() << "  " << #info << ": " << info_value << sys::io::endl

    DEV_INFO(CL_DEVICE_NAME);
    DEV_INFO(CL_DEVICE_OPENCL_C_VERSION);
    DEV_INFO(CL_DEVICE_PROFILE);
    DEV_INFO(CL_DEVICE_VENDOR);
    DEV_INFO(CL_DEVICE_VERSION);
    DEV_INFO(CL_DRIVER_VERSION);

    engine.out() << "  CL_DEVICE_EXTENSIONS:" << sys::io::endl;
    dev.getInfo(CL_DEVICE_EXTENSIONS, &info_value);
    std::istringstream dev_exts(info_value);
    std::string dev_ext;

    bool supports_3d_image_write = false;
    while(std::getline(dev_exts, dev_ext, ' ')) {
        engine.out() << "    " << dev_ext << sys::io::endl;
        if (dev_ext == "cl_khr_3d_image_writes")
            supports_3d_image_write = true;
    }

    engine.out() << "  END EXTENSIONS" << sys::io::endl;

    if (!supports_3d_image_write) {
        ERR("cl_khr_3d_image_writes not supported");
        return;
    }

    cl_q = cl::CommandQueue(cl_ctx, dev, 0, &err);
    if (err != CL_SUCCESS) {
        ERR("creating command queue failed");
        return;
    }
        

    ev.info.success = true;
}

void Anim::link(ge::Engine& e) {
    e.events().animate.reg(ge::makeEventHandler(this, &Anim::animate));
    e.events().render.reg(ge::makeEventHandler(this, &Anim::renderScene));
    e.window().events().windowResized.reg(ge::makeEventHandler(this, &Anim::handleWindowResized));
}

void Anim::animate(const ge::Event<ge::AnimationEvent>&) {
    // empty
}

void Anim::renderScene(const ge::Event<ge::RenderEvent>& ev) {
    ge::Engine& e = ev.info.engine;
    real time = e.gameLoop().gameTime() + ev.info.interpolation * e.gameLoop().frameDuration();

    glt::RenderManager& rm = engine.renderManager();

    if (time >= time_print_fps) {
        time_print_fps = time + 1.f;

        #define INV(x) (((x) * (x)) <= 0 ? -1 : 1.0 / (x))
        glt::FrameStatistics fs = engine.renderManager().frameStatistics();
        double fps = INV(fs.avg);
        double min = INV(fs.max);
        double max = INV(fs.min);
        double avg = INV(fs.avg);
        engine.out() << "Timings (FPS/Render Avg/Render Min/Render Max): " << fps << "; " << avg << "; " << min << "; " << max << sys::io::endl;
    }
}

void Anim::handleWindowResized(const ge::Event<ge::WindowResized>&) {
    // empty
}

int main(int argc, char *argv[]) {
    Anim anim;
    ge::EngineOptions opts;

    opts.parse(&argc, &argv);
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::init));
    opts.inits.reg(ge::Init, ge::makeEventHandler(&anim, &Anim::initCL));
    return anim.engine.run(opts);
}
