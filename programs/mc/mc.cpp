#define CL_HPP_MINIMUM_OPENCL_VERSION 200
#define CL_HPP_TARGET_OPENCL_VERSION 200

#include <CL/opencl.hpp>

#include "ge/Camera.hpp"
#include "ge/Engine.hpp"
#include "ge/MouseLookPlugin.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/GLSLPreprocessor.hpp"
#include "glt/Mesh.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/TextureRenderTarget.hpp"
#include "glt/geometric.hpp"
#include "glt/utils.hpp"
#include "math/mat4.hpp"
#include "math/real.hpp"
#include "math/vec4.hpp"
#include "sys/measure.hpp"
#include "util/range.hpp"

#include "shaders/shader_constants.h"

#include <array>
#include <sstream>
#include <string>
#include <vector>

template<size_t N>
using cl_size = std::array<cl::size_type, N>;

#ifdef HU_OS_POSIX
#    include <GL/glx.h>
#endif

#define CL_ERR(msg)                                                            \
    do {                                                                       \
        if (cl_err != CL_SUCCESS) {                                            \
            ERR(msg);                                                          \
            return;                                                            \
        }                                                                      \
    } while (0)

static const size_t DEFAULT_N = 128;

using namespace math;

DEF_GL_MAPPED_TYPE(Vertex, (vec3_t, position), (vec3_t, normal))

struct MCState
{
    vec3_t cube_origin{}; // lower left corner on backside
    vec3_t cube_dim{};

    bool init{};

    int32_t count[8]{};
    int32_t num_tris{};

    GLuint vao{};
    GLuint vbo{};
    cl::BufferGL vbo_buf;

    cl::Image3D volume;
    std::vector<cl::Image3D> images;
    cl::Event readHistoJob;
    cl::Event writeVBOJob;
};

#define MC_SIZE_X 2
#define MC_SIZE_Y 2
#define MC_SIZE_Z 2
#define NUM_MC (MC_SIZE_X * MC_SIZE_Y * MC_SIZE_Z)

struct Anim
{
    ge::Engine engine;
    real time_print_fps{};

    ge::Camera camera;
    ge::MouseLookPlugin mouse_look;

    size_t N{};
    cl::Context cl_ctx;
    cl::CommandQueue cl_q;
    cl::Device cl_device;

    cl::Program cl_program;
    cl::Kernel kernel_generateSphereVolume;
    cl::Kernel kernel_computeHistogramBaseLevel;
    cl::Kernel kernel_computeHistogramLevel;
    cl::Kernel kernel_histogramTraversal;

    size_t mdl_num_tris{};
    char *mdl_base{};
    char *mdl_data{};

    MCState mc[NUM_MC];

    double sum_tris{};
    double max_tris{};
    uint32_t num_renders{};

    void init(const ge::Event<ge::InitEvent> & /*ev*/);
    void initCL(const ge::Event<ge::InitEvent> & /*ev*/);
    void initCLKernels(bool *success);
    void initMC(MCState &mc, vec3_t orig, vec3_t dim, bool *success);

    void computeVolumeData(MCState &mc, real time);
    void computeHistogram(MCState &mc);
    void constructVertices0(MCState &mc);
    void constructVertices1(MCState &mc);

    void renderMC(MCState &mc,
                  glt::ShaderProgram & /*program*/,
                  vec3_t ecLight);

    void link(ge::Engine &e);
    void animate(const ge::Event<ge::AnimationEvent> & /*unused*/);
    void renderScene(const ge::Event<ge::RenderEvent> & /*ev*/);

    void handleWindowResized(const ge::Event<ge::WindowResized> &ev);
};

void
Anim::init(const ge::Event<ge::InitEvent> &ev)
{
    link(ev.info.engine);
    time_print_fps = 0;

    engine.gameLoop().ticks(100);
    //    engine.gameLoop().syncDraw(true);
    engine.gameLoop().pause();

    //    GL_CALL(glDisable, GL_CULL_FACE);
    GL_CALL(glEnable, GL_CULL_FACE);

    ev.info.success = true;
}

static void CL_CALLBACK
print_cl_error(const char *msg,
               const void * /*unused*/,
               ::size_t /*unused*/,
               void * /*unused*/)
{
    sys::io::stdout() << msg << "\n";
}

static cl::Context
createCLGLContext(cl::Platform &platform, cl_int *err)
{

    cl_context_properties props[] = {
#if HU_OS_WINDOWS_P
        CL_GL_CONTEXT_KHR,
        cl_context_properties(wglGetCurrentContext()),
        CL_WGL_HDC_KHR,
        cl_context_properties(wglGetCurrentDC()),
        CL_CONTEXT_PLATFORM,
        cl_context_properties((platform) ()),
#elif HU_OS_POSIX_P
        CL_GL_CONTEXT_KHR,
        cl_context_properties(glXGetCurrentContext()),
        CL_GLX_DISPLAY_KHR,
        cl_context_properties(glXGetCurrentDisplay()),
        CL_CONTEXT_PLATFORM,
        cl_context_properties((platform) ()),
#else
#    error "unknown system"
#endif
        0,
        0
    };

    return { CL_DEVICE_TYPE_GPU, props, print_cl_error, nullptr, err };
}

template<typename T>
T
log2(T x)
{
    T l = 1;
    T i = 0;
    while (l < x) {
        l *= 2;
        ++i;
    }

    return i;
}

void
Anim::initCL(const ge::Event<ge::InitEvent> &ev)
{

    ev.info.engine.enablePlugin(mouse_look);
    mouse_look.camera(&camera);
    ev.info.engine.enablePlugin(camera);
    camera.frame().origin = vec3(0.f);

    N = DEFAULT_N;

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    engine.out() << "found " << platforms.size() << " CL platforms"
                 << "\n";
    if (platforms.empty())
        return;
    platforms.resize(1);

    cl::Platform &platform = platforms[0];
    std::string info_value;
    engine.out() << "selecting CL platform:\n";

#define PLAT_INFO(info)                                                        \
    platform.getInfo(info, &info_value);                                       \
    engine.out() << "  " << #info << ": " << info_value << "\n"

    PLAT_INFO(CL_PLATFORM_NAME);
    PLAT_INFO(CL_PLATFORM_PROFILE);
    PLAT_INFO(CL_PLATFORM_VENDOR);
    PLAT_INFO(CL_PLATFORM_VERSION);
#undef PLAT_INFO

    engine.out() << "  CL_PLATFORM_EXTENSIONS:\n";
    platform.getInfo(CL_PLATFORM_EXTENSIONS, &info_value);
    std::istringstream exts(info_value);
    std::string ext;
    while (std::getline(exts, ext, ' '))
        engine.out() << "    " << ext << "\n";

    engine.out() << "  END EXTENSIONS\n";

    cl_int cl_err;
    cl_ctx = createCLGLContext(platform, &cl_err);
    if (cl_err != CL_SUCCESS) {
        ERR("createing shared OpenCL/OpenGL context failed");
        return;
    }

    engine.out() << "created CL context\n";

    std::vector<cl::Device> devices = cl_ctx.getInfo<CL_CONTEXT_DEVICES>();
    if (devices.empty())
        return;
    devices.resize(1);
    engine.out() << "selecting CL device:\n";
    cl_device = devices[0];

#define DEV_INFO(info)                                                         \
    cl_device.getInfo(info, &info_value);                                      \
    engine.out() << "  " << #info << ": " << info_value << "\n"

    DEV_INFO(CL_DEVICE_NAME);
    DEV_INFO(CL_DEVICE_OPENCL_C_VERSION);
    DEV_INFO(CL_DEVICE_PROFILE);
    DEV_INFO(CL_DEVICE_VENDOR);
    DEV_INFO(CL_DEVICE_VERSION);
    DEV_INFO(CL_DRIVER_VERSION);

    engine.out() << "  CL_DEVICE_EXTENSIONS:\n";
    cl_device.getInfo(CL_DEVICE_EXTENSIONS, &info_value);
    std::istringstream dev_exts(info_value);
    std::string dev_ext;

    bool supports_3d_image_write = false;
    while (std::getline(dev_exts, dev_ext, ' ')) {
        engine.out() << "    " << dev_ext << "\n";
        if (dev_ext == "cl_khr_3d_image_writes")
            supports_3d_image_write = true;
    }

    engine.out() << "  END EXTENSIONS\n";

    if (!supports_3d_image_write) {
        ERR("cl_khr_3d_image_writes not supported");
        return;
    }

    cl_q =
      cl::CommandQueue(cl_ctx, cl_device, CL_QUEUE_PROFILING_ENABLE, &cl_err);
    CL_ERR("creating command queue failed");

    bool ok = false;
    initCLKernels(&ok);
    if (!ok)
        return;

    vec3_t dim =
      real(2) * recip(vec3(real(MC_SIZE_X), real(MC_SIZE_Y), real(MC_SIZE_Z)));

    for (int i = 0; i < MC_SIZE_X; ++i) {
        for (int j = 0; j < MC_SIZE_Y; ++j) {
            for (int k = 0; k < MC_SIZE_Z; ++k) {
                vec3_t corner =
                  vec3(real(i), real(j), real(k)) * dim - vec3(real(1));
                int idx = i * (MC_SIZE_Y * MC_SIZE_Z) + j * MC_SIZE_Z + k;
                ok = false;
                initMC(mc[idx], corner, dim, &ok);
                if (!ok)
                    return;
            }
        }
    }

    ev.info.success = true;
}

void
Anim::initCLKernels(bool *success)
{
    cl_int cl_err;

    sys::io::HandleError err;
    auto source_code =
      sys::io::readFile(engine.out(), "programs/mc/cl/program.cl", err);

    if (err != sys::io::HandleError::OK) {
        ERR("failed reading CL program");
        return;
    }

    std::string source_str(source_code.data(), source_code.size());
    cl::Program::Sources source{ source_str };
    cl_program = cl::Program(cl_ctx, source, &cl_err);
    CL_ERR("creating program failed");

    std::stringstream num;
    num << N;
    std::vector<cl::Device> devices;
    devices.push_back(cl_device);
    cl_err = cl_program.build(
      devices, (std::string("-I programs/mc/ -DSIZE=") + num.str()).c_str());
    engine.out() << "building cl program: "
                 << (cl_err == CL_SUCCESS ? "success" : "failed") << ", log:\n";
    engine.out() << cl_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(cl_device)
                 << "\n";

    if (cl_err != CL_SUCCESS)
        return;

    kernel_generateSphereVolume =
      cl::Kernel(cl_program, "generateSphereVolume", &cl_err);
    CL_ERR("creating kernel failed: generateSphereVolume");
    kernel_computeHistogramBaseLevel =
      cl::Kernel(cl_program, "computeHistogramBaseLevel", &cl_err);
    CL_ERR("creating kernel failed: computeHistogramBaseLevel");
    kernel_computeHistogramLevel =
      cl::Kernel(cl_program, "computeHistogramLevel", &cl_err);
    CL_ERR("creating kernel failed: computeHistogramLevel");
    kernel_histogramTraversal =
      cl::Kernel(cl_program, "histogramTraversal", &cl_err);
    CL_ERR("creating kernel failed: histogramTraversal");

    *success = true;
}

void
Anim::initMC(MCState &cur_mc, vec3_t orig, vec3_t dim, bool *success)
{
#define IS_POWER_OF_2(x) ((((x) -1) & (x)) == 0)
    int cl_err;

    ASSERT(IS_POWER_OF_2(N), "N has to be a power of 2!");
    ASSERT(N <= 512, "N has to be <= 512");

#undef IS_POWER_OF_2

    cur_mc.init = false;
    cur_mc.vbo = 0;
    cur_mc.vao = 0;

    cur_mc.cube_origin = orig;
    cur_mc.cube_dim = dim;

    cur_mc.volume = cl::Image3D(cl_ctx,
                                CL_MEM_READ_WRITE,
                                cl::ImageFormat(CL_R, CL_HALF_FLOAT),
                                N + 2,
                                N + 2,
                                N + 2,
                                0,
                                0,
                                nullptr,
                                &cl_err);
    CL_ERR("creating volume 3d image failed");

    cur_mc.images.clear();

    cur_mc.images.emplace_back(cl_ctx,
                               CL_MEM_READ_WRITE,
                               cl::ImageFormat(CL_RG, CL_UNSIGNED_INT8),
                               N,
                               N,
                               N,
                               0,
                               0,
                               nullptr,
                               &cl_err);
    CL_ERR("creating histogram base level failed");

    auto sz = N / 2;
    int max_size = 5;

    // apparently creating a 3d texture of size 1x1x1 fails
    // while (sz > 0) {
    while (sz > 1) {
        engine.out() << "sz = " << sz << "\n";
        max_size *= 8;
        cl_channel_type type;
        if (max_size >= (1 << 16))
            type = CL_UNSIGNED_INT32;
        else if (max_size >= (1 << 8))
            type = CL_UNSIGNED_INT16;
        else
            type = CL_UNSIGNED_INT8;
        cur_mc.images.emplace_back(cl_ctx,
                                   CL_MEM_READ_WRITE,
                                   cl::ImageFormat(CL_R, type),
                                   sz,
                                   sz,
                                   sz,
                                   0,
                                   0,
                                   nullptr,
                                   &cl_err);
        CL_ERR("creating histogram level failed");
        sz /= 2;
    }

    *success = true;
}

void
Anim::computeVolumeData(MCState &cur_mc, real time)
{
    mat4_t M = mat4();
    cur_mc.init = true;

    M *= glt::translateTransform(cur_mc.cube_origin);
    M *= glt::scaleTransform(cur_mc.cube_dim);
    M *= glt::scaleTransform(real(1) / real(N - 2));

#define fract(x) ((x) -math::floor(x))
#define tri(t) math::abs(2 * fract((t) / (2 * math::PI)) - 1)

    time *= real(0.5);
    const vec4_t C = vec4(-1 * tri(math::cos(time) * 0.1),
                          real(0.2) * math::cos(time * real(0.11)),
                          math::sin(time * real(0.1)),
                          0);

    kernel_generateSphereVolume.setArg(0, cur_mc.volume);
    kernel_generateSphereVolume.setArg(1, M);
    kernel_generateSphereVolume.setArg(2, C);

    cl_q.enqueueNDRangeKernel(kernel_generateSphereVolume,
                              cl::NullRange,
                              cl::NDRange(N + 2, N + 2, N + 2),
                              cl::NullRange);
}

void
Anim::computeHistogram(MCState &cur_mc)
{
    if (!cur_mc.init)
        return;

    kernel_computeHistogramBaseLevel.setArg(0, cur_mc.images[0]);
    kernel_computeHistogramBaseLevel.setArg(1, cur_mc.volume);

    cl_q.enqueueNDRangeKernel(kernel_computeHistogramBaseLevel,
                              cl::NullRange,
                              cl::NDRange(N, N, N),
                              cl::NullRange);

    auto sz = N;
    size_t i = 0;
    while (sz > 2) {
        sz /= 2;

        kernel_computeHistogramLevel.setArg(0, cur_mc.images[i]);
        kernel_computeHistogramLevel.setArg(1, cur_mc.images[i + 1]);

        cl_q.enqueueNDRangeKernel(kernel_computeHistogramLevel,
                                  cl::NullRange,
                                  cl::NDRange(sz, sz, sz),
                                  cl::NullRange);

        ++i;
    }

    ASSERT(i + 1 == cur_mc.images.size());

    auto origin = cl_size<3>{ 0, 0, 0 };
    auto cube = cl_size<3>{ 2, 2, 2 };
    cl_q.enqueueReadImage(cur_mc.images[cur_mc.images.size() - 1],
                          CL_FALSE,
                          origin,
                          cube,
                          0,
                          0,
                          cur_mc.count,
                          nullptr,
                          &cur_mc.readHistoJob);
}

void
Anim::constructVertices0(MCState &cur_mc)
{
    if (!cur_mc.init)
        return;
    cur_mc.readHistoJob.wait();

    cur_mc.num_tris = 0;
    for (int i : cur_mc.count)
        cur_mc.num_tris += i;

    if (cur_mc.num_tris == 0)
        return;

    GL_CALL(glGenBuffers, 1, &cur_mc.vbo);
    GL_CALL(glGenVertexArrays, 1, &cur_mc.vao);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, cur_mc.vbo);
    GL_CALL(glBufferData,
            GL_ARRAY_BUFFER,
            cur_mc.num_tris * 3 * 2 * sizeof(vec3_t),
            nullptr,
            GL_STREAM_DRAW);
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

    cur_mc.vbo_buf = cl::BufferGL(cl_ctx, CL_MEM_WRITE_ONLY, cur_mc.vbo);
}

void
Anim::constructVertices1(MCState &cur_mc)
{
    if (cur_mc.num_tris == 0 || !cur_mc.init)
        return;

    size_t i;
    for (i = 0; i < cur_mc.images.size(); ++i)
        kernel_histogramTraversal.setArg(i, cur_mc.images[i]);

    kernel_histogramTraversal.setArg(i, cur_mc.volume);
    ++i;
    kernel_histogramTraversal.setArg(i, cur_mc.vbo_buf);
    ++i;
    kernel_histogramTraversal.setArg(i, cur_mc.num_tris);

    int global_work_size = (cur_mc.num_tris / 64 + 1) * 64;
    cl_q.enqueueNDRangeKernel(kernel_histogramTraversal,
                              cl::NullRange,
                              cl::NDRange(global_work_size),
                              cl::NDRange(64),
                              nullptr);
}

void
Anim::renderMC(MCState &cur_mc, glt::ShaderProgram &program, vec3_t ecLight)
{

    if (cur_mc.num_tris == 0 || !cur_mc.init)
        return;

    glt::RenderManager &rm = engine.renderManager();
    glt::GeometryTransform &gt = rm.geometryTransform();

    mat4_t M = mat4();

    M *= glt::translateTransform(cur_mc.cube_origin);
    M *= glt::scaleTransform(cur_mc.cube_dim);
    M *= glt::scaleTransform(real(1) / real(N - 2));

    gt.dup();
    gt.concat(M);

    glt::Uniforms(program)
      .optional("mvpMatrix", gt.mvpMatrix())
      .optional("mvMatrix", gt.mvMatrix())
      .optional("normalMatrix", gt.normalMatrix())
      .optional("ecLight", ecLight)
      .optional("albedo", vec3(real(1)));

    gt.pop();

    auto struct_info = Vertex::gl::struct_info::info;

    for (const auto [i, a] : enumerate(struct_info.fields)) {
        GL_CALL(glVertexArrayVertexAttribOffsetEXT,
                cur_mc.vao,
                cur_mc.vbo,
                GLuint(i),
                a.type_info.arity,
                glt::toGLScalarType(a.type_info.scalar_type),
                a.type_info.normalized ? GL_TRUE : GL_FALSE,
                struct_info.size,
                GLintptr(a.offset));
        GL_CALL(glEnableVertexArrayAttribEXT, cur_mc.vao, GLuint(i));
    }

    GL_CALL(glBindVertexArray, cur_mc.vao);
    GL_CALL(glDrawArrays, GL_TRIANGLES, 0, cur_mc.num_tris * 3);
    GL_CALL(glBindVertexArray, 0);

    GL_CALL(glDeleteBuffers, 1, &cur_mc.vbo);
    GL_CALL(glDeleteVertexArrays, 1, &cur_mc.vao);
}

void
Anim::link(ge::Engine &e)
{
    e.events().animate.reg(*this, &Anim::animate);
    e.events().render.reg(*this, &Anim::renderScene);
    e.window().events().windowResized.reg(*this, &Anim::handleWindowResized);
}

void
Anim::animate(const ge::Event<ge::AnimationEvent> & /*unused*/)
{
    // empty
}

void
Anim::renderScene(const ge::Event<ge::RenderEvent> &ev)
{
    ge::Engine &e = ev.info.engine;
    real time = e.gameLoop().tickTime();

    glt::RenderManager &rm = engine.renderManager();
    rm.activeRenderTarget()->clear();
    glt::GeometryTransform &gt = rm.geometryTransform();

    auto program = engine.shaderManager().program("render");
    ASSERT(program);

    program->use();

    point3_t wcLight = vec3(1, 1, 1);
    point3_t ecLight = vec3(transform(gt.viewMatrix(), vec4(wcLight, real(1))));

#define TIME(...) __VA_ARGS__
    TIME(for (auto &i : mc) computeVolumeData(i, time));
    TIME(for (auto &i : mc) computeHistogram(i));
    TIME(for (auto &i : mc) constructVertices0(i));

    std::vector<cl::Memory> gl_objs;
    for (auto &i : mc)
        if (i.num_tris > 0 && i.init)
            gl_objs.push_back(i.vbo_buf);

    cl_q.enqueueAcquireGLObjects(&gl_objs);
    for (auto &i : mc)
        constructVertices1(i);

    cl_q.enqueueReleaseGLObjects(&gl_objs);
    cl_q.finish();

    TIME(for (auto &i : mc) renderMC(i, *program, ecLight));

    double ntris = 0;
    for (auto &i : mc)
        ntris += i.num_tris;

    if (ntris > max_tris)
        max_tris = ntris;
    sum_tris += ntris;
    ++num_renders;

    time = engine.gameLoop().realTime();
    if (time >= time_print_fps) {
        time_print_fps = time + 1.f;

#define INV(x) (((x) * (x)) <= 0 ? -1 : 1.0 / (x))
        glt::FrameStatistics fs = engine.renderManager().frameStatistics();
        double fps = INV(fs.avg);
        double min = INV(fs.max);
        double max = INV(fs.min);
        double avg = INV(fs.avg);
        engine.out() << "Timings (FPS/Render Avg/Render Min/Render Max): "
                     << fps << "; " << avg << "; " << min << "; " << max
                     << "\n";

        double avg_tris = sum_tris / num_renders;

        engine.out() << "Avg Tris: " << avg_tris << " Max Tris: " << max_tris
                     << "\n";

        max_tris = 0;
        num_renders = 0;
        sum_tris = 0;
    }
}

void
Anim::handleWindowResized(const ge::Event<ge::WindowResized> & /*unused*/)
{
    // empty
}

int
main(int argc, char *argv[])
{
    Anim anim;
    ge::EngineOptions opts;

    anim.engine.setDevelDataDir(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR));
    opts.parse(&argc, &argv);
    opts.inits.reg(ge::Init, anim, &Anim::init);
    opts.inits.reg(ge::Init, anim, &Anim::initCL);
    return anim.engine.run(opts);
}
