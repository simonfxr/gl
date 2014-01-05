#include "ge/Engine.hpp"

#include "math/real.hpp"
#include "math/vec4.hpp"
#include "math/mat4.hpp"

#include "glt/ShaderProgram.hpp"
#include "glt/Mesh.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/TextureRenderTarget.hpp"
#include "glt/utils.hpp"
#include "glt/GLSLPreprocessor.hpp"

#include "sys/measure.hpp"

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

#define CL_ERR(msg) do { if (cl_err != CL_SUCCESS) { ERR(msg); return; } } while (0)

static const size DEFAULT_N = 256;

using namespace math;

struct Vertex {
    vec2_t position;
};

DEFINE_VERTEX_DESC(Vertex,
                   VERTEX_ATTR(Vertex, position));

struct Anim {
    ge::Engine engine;
    real time_print_fps;

    size N;
    cl::Context cl_ctx;
    cl::CommandQueue cl_q;
    cl::Device cl_device;

    cl::Program cl_program;
    cl::Kernel kernel_generateSphereVolume;
    cl::Kernel kernel_computeHistogramBaseLevel;
    cl::Kernel kernel_computeHistogramLevel;
    cl::Kernel kernel_histogramTraversal;

    cl::Image3D volumeData;
    std::vector<cl::Image3D> images;
    
    void init(const ge::Event<ge::InitEvent>&);
    void initCL(const ge::Event<ge::InitEvent>&);
    void initCLKernels(bool *success);
    void initCLData(bool *success);

    size computeHistogram();
    void constructVertices(size num_tris);
    
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

template <typename T>
T log2(T x) {
    T l = 1;
    T i = 0;
    while (l < x) {
        l *= 2;
        ++i;
    }

    return i;
}

void Anim::initCL(const ge::Event<ge::InitEvent>& ev) {

    N = DEFAULT_N;
    
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

    cl_int cl_err;
    cl_ctx = createCLGLContext(platform, &cl_err);
    if (cl_err != CL_SUCCESS) {
        ERR("createing shared OpenCL/OpenGL context failed");
        return;
    }

    engine.out() << "created CL context" << sys::io::endl;
    
    std::vector <cl::Device> devices = cl_ctx.getInfo<CL_CONTEXT_DEVICES>();
    if (devices.size() < 1)
        return;
    devices.resize(1);
    engine.out() << "selecting CL device:" << sys::io::endl;
    cl_device = devices[0];

#define DEV_INFO(info)                                                  \
    cl_device.getInfo(info, &info_value);                               \
    engine.out() << "  " << #info << ": " << info_value << sys::io::endl

    DEV_INFO(CL_DEVICE_NAME);
    DEV_INFO(CL_DEVICE_OPENCL_C_VERSION);
    DEV_INFO(CL_DEVICE_PROFILE);
    DEV_INFO(CL_DEVICE_VENDOR);
    DEV_INFO(CL_DEVICE_VERSION);
    DEV_INFO(CL_DRIVER_VERSION);

    engine.out() << "  CL_DEVICE_EXTENSIONS:" << sys::io::endl;
    cl_device.getInfo(CL_DEVICE_EXTENSIONS, &info_value);
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

    cl_q = cl::CommandQueue(cl_ctx, cl_device, 0, &cl_err);
    CL_ERR("creating command queue failed");

    bool ok = false;
    initCLKernels(&ok);
    if (!ok) return;
    
    ok = false;
    initCLData(&ok);
    if (!ok) return;

    ev.info.success = true;
}

void Anim::initCLKernels(bool *success) {
    cl_int cl_err;
    char *source_code;
    uint32 code_size;

    if (!glt::readFile(engine.out(), "programs/mc/cl/program.cl", source_code, code_size)) {
        ERR("failed reading CL program");
        return;
    }

    cl::Program::Sources source(1, std::make_pair(source_code, code_size));
    cl_program = cl::Program(cl_ctx, source, &cl_err);
    CL_ERR("creating program failed");

    std::stringstream num;
    num << N;
    std::vector<cl::Device> devices;
    devices.push_back(cl_device);
    cl_err = cl_program.build(devices, (std::string("-I programs/mc/ -DSIZE=") + num.str()).c_str());
    engine.out() << "building cl program: " << (cl_err == CL_SUCCESS ? "success" : "failed") << ", log:" << sys::io::endl;
    engine.out() << cl_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(cl_device) << sys::io::endl;

    if (cl_err != CL_SUCCESS)
        return;

    kernel_generateSphereVolume = cl::Kernel(cl_program, "generateSphereVolume", &cl_err);
    CL_ERR("creating kernel failed: generateSphereVolume");
    kernel_computeHistogramBaseLevel = cl::Kernel(cl_program, "computeHistogramBaseLevel", &cl_err);
    CL_ERR("creating kernel failed: computeHistogramBaseLevel");
    kernel_computeHistogramLevel = cl::Kernel(cl_program, "computeHistogramLevel", &cl_err);
    CL_ERR("creating kernel failed: computeHistogramLevel");
    kernel_histogramTraversal = cl::Kernel(cl_program, "histogramTraversal", &cl_err);
    CL_ERR("creating kernel failed: histogramTraversal");

    *success = true;
}


void Anim::initCLData(bool *success) {

#define IS_POWER_OF_2(x) (((x - 1) & x) == 0)
    int cl_err;
    
    ASSERT_MSG(IS_POWER_OF_2(N), "N has to be a power of 2!");
    ASSERT_MSG(N <= 256, "N has to be <= 256");        

    volumeData = cl::Image3D(cl_ctx, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_UNSIGNED_INT8), N, N, N, 0, 0, 0, &cl_err);
    CL_ERR("creating volume 3d image failed");

    images.push_back(cl::Image3D(cl_ctx, CL_MEM_READ_WRITE, cl::ImageFormat(CL_RGBA, CL_UNSIGNED_INT8), N, N, N, 0, 0, 0, &cl_err));
    CL_ERR("creating histogram base level failed");
    
    size sz = N / 2;
    int max_size = 5;

    // apparently creating a 3d texture of size 1x1x1 fails
    // while (sz > 0) {
    while (sz > 1) {
        engine.out() << "sz = " << sz << sys::io::endl;
        max_size *= 8;
        cl_channel_type type;
        if (max_size >= (1 << 16))
            type = CL_UNSIGNED_INT32;
        else if (max_size >= (1 << 8))
            type = CL_UNSIGNED_INT16;
        else
            type = CL_UNSIGNED_INT8;
        images.push_back(cl::Image3D(cl_ctx, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, type), sz, sz, sz, 0, 0, 0, &cl_err));
        CL_ERR("creating histogram level failed");
        sz /= 2;
    }

    kernel_generateSphereVolume.setArg(0, volumeData);
    kernel_generateSphereVolume.setArg(1, mat4());
    kernel_generateSphereVolume.setArg(2, vec4(real(N) * 0.5f));
    kernel_generateSphereVolume.setArg(3, real(N) * 0.25f);

    cl_q.enqueueNDRangeKernel(
        kernel_generateSphereVolume,
        cl::NullRange, cl::NDRange(N, N, N), cl::NullRange);

    *success = true;
}


size Anim::computeHistogram() {

    kernel_computeHistogramBaseLevel.setArg(0, images[0]);
    kernel_computeHistogramBaseLevel.setArg(1, volumeData);
    kernel_computeHistogramBaseLevel.setArg(2, 128);

    cl_q.enqueueNDRangeKernel(
        kernel_computeHistogramBaseLevel,
        cl::NullRange, cl::NDRange(N, N, N), cl::NullRange);

    size sz = N;
    defs::index i = 0;
    while (sz > 2) {
        sz /= 2;

        kernel_computeHistogramLevel.setArg(0, images[i]);
        kernel_computeHistogramLevel.setArg(1, images[i+1]);

        cl_q.enqueueNDRangeKernel(
            kernel_computeHistogramLevel,
            cl::NullRange, cl::NDRange(sz, sz, sz), cl::NullRange);

        ++i;
    }

    ASSERT(i + 1 == SIZE(images.size()));

    int32 count[8];
    cl::size_t<3> origin;
    cl::size_t<3> cube;
    origin[0] = origin[1] = origin[2] = 0;
    cube[0] = cube[1] = cube[2] = 2;
    cl_q.enqueueReadImage(images[images.size() - 1], CL_FALSE, origin, cube, 0, 0, count);
    cl_q.finish();

    int32 total = 0;
    for (defs::index i = 0; i < 8; ++i)
        total += count[i];

    engine.out() << "number of triangles: " << total << sys::io::endl;

    return total;
}

void Anim::constructVertices(size num_tris) {

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, num_tris * 3 * 2 * sizeof(vec3_t), 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    defs::index i;
    for (i = 0; i < SIZE(images.size()); ++i)
        kernel_histogramTraversal.setArg(i, images[i]);

    cl::BufferGL vbo_buf(cl_ctx, CL_MEM_WRITE_ONLY, vbo);
    kernel_histogramTraversal.setArg(i, vbo_buf); ++i;
    kernel_histogramTraversal.setArg(i, 128); ++i;
    kernel_histogramTraversal.setArg(i, num_tris);

    std::vector<cl::Memory> gl_objs;
    gl_objs.push_back(vbo_buf);
    cl_q.enqueueAcquireGLObjects(&gl_objs);

    int global_work_size = (num_tris / 64 + 1) * 64;
    cl_q.enqueueNDRangeKernel(
        kernel_histogramTraversal,
        cl::NullRange, cl::NDRange(global_work_size),
        cl::NDRange(64));

    cl_q.enqueueReleaseGLObjects(&gl_objs);
    cl_q.finish();

    glDeleteBuffers(1, &vbo);
}

void Anim::link(ge::Engine& e) {
    e.events().animate.reg(ge::makeEventHandler(this, &Anim::animate));
    e.events().render.reg(ge::makeEventHandler(this, &Anim::renderScene));
    e.window().events().windowResized.reg(ge::makeEventHandler(this, &Anim::handleWindowResized));
}

void Anim::animate(const ge::Event<ge::AnimationEvent>&) {
    size num_tris;
    time_op(num_tris = computeHistogram());
    time_op(constructVertices(num_tris));
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
