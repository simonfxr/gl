#ifdef _WIN32
#    define _CRT_SECURE_NO_WARNINGS 1
#endif

#define MESH_CUBEMESH
// #define MESH_MESH

// #define RENDER_GLOW 1
#define RENDER_NOGLOW 1

#include "defs.h"

#include "ge/Camera.hpp"
#include "ge/Engine.hpp"
#include "ge/MouseLookPlugin.hpp"
#include "ge/Timer.hpp"
#include "glt/Frame.hpp"
#include "glt/Mesh.hpp"
#include "glt/TextureRenderTarget.hpp"
#include "glt/Uniforms.hpp"
#include "glt/color.hpp"
#include "glt/primitives.hpp"
#include "glt/utils.hpp"
#include "math/mat2.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/real.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "sys/fs.hpp"
#ifdef MESH_CUBEMESH
#    include "glt/CubeMesh.hpp"
#endif

#include "dump_bmp.h"

#include <vector>

#ifdef MESH_CUBEMESH
template<typename T>
using MeshOf = glt::Mesh<T>;

template<typename T>
using CubeMeshOf = glt::CubeMesh<T>;
#else
template<typename T>
using MeshOf = glt::Mesh<T>;

template<typename T>
using CubeMeshOf = glt::Mesh<T>;
#endif

#include "shaders/glow_pass_constants.h"

#if !(defined(RENDER_GLOW) || defined(RENDER_NOGLOW))
#    error "no glowmode defined"
#endif

using namespace math;
using namespace ge;

DEF_GL_MAPPED_TYPE(Vertex,
                   (math::point3_t, position),
                   (math::direction3_t, normal))

DEF_GL_MAPPED_TYPE(Vertex2,
                   (math::vec3_t, position),
                   (math::vec3_t, normal),
                   (math::vec2_t, texCoord))

DEF_GL_MAPPED_TYPE(ScreenVertex,
                   (math::vec3_t, position),
                   (math::vec3_t, normal))

int32_t
parse_sply(const char *filename, CubeMeshOf<Vertex> &model);

static const point3_t LIGHT_CENTER_OF_ROTATION = vec3(0.f, 15.f, 0.f);
static const float LIGHT_ROTATION_RAD = 15.f;

static const uint32_t SHADE_MODE_AMBIENT = 1;
static const uint32_t SHADE_MODE_DIFFUSE = 2;
static const uint32_t SHADE_MODE_SPECULAR = 4;
static const uint32_t SHADE_MODE_DEFAULT =
  SHADE_MODE_AMBIENT | SHADE_MODE_DIFFUSE | SHADE_MODE_SPECULAR;

struct MaterialProperties
{
    float ambientContribution;
    float diffuseContribution;
    float specularContribution;
    float shininess;
    float glow;
};

struct Teapot
{
    glt::Frame frame;
    MaterialProperties material{};
    glt::color color;
};

struct Anim
{
    ge::Engine &engine;
    ge::Camera camera;
    ge::MouseLookPlugin mouse_look;
    std::vector<float> glow_kernel;

    CubeMeshOf<Vertex> groundModel;
    CubeMeshOf<Vertex> teapotModel;
    MeshOf<Vertex> sphereModel;
    CubeMeshOf<Vertex2> cubeModel;
    glt::CubeMesh<ScreenVertex> screenQuad;

    glt::TextureSampler woodTexture;

    Teapot teapot1;
    Teapot teapot2;

    bool wireframe_mode{};

    real light_angular_position{};
    real light_rotation_speed{};

    real gamma_correction{};

    vec3_t light{};
    vec3_t ecLight{};

    uint32_t shade_mode{};

    bool use_spotlight{};
    bool spotlight_smooth{};
    direction3_t ec_spotlight_dir{};

    std::shared_ptr<glt::TextureRenderTarget> tex_render_target;
    std::shared_ptr<glt::TextureRenderTarget> glow_render_target_src;
    std::shared_ptr<glt::TextureRenderTarget> glow_render_target_dst;

    std::shared_ptr<Timer> fpsTimer;

    std::string data_dir;

    explicit Anim(ge::Engine &e) : engine(e), glow_kernel(KERNEL_SIZE) {}

    void init(const Event<InitEvent> & /*ev*/);
    void loadResources(const std::string &dir);

    void link(const Event<InitEvent> & /*e*/);
    void animate(const Event<AnimationEvent> & /*unused*/);
    void renderScene(const Event<RenderEvent> & /*e*/);
    vec3_t lightPosition(real interpolation);
    void setupTeapotShader(const std::string &prog,
                           const vec4_t &surfaceColor,
                           const MaterialProperties &mat);
    void renderTeapot(const Teapot &teapot);
    void renderGround();
    void renderLight();
    void renderTable(const std::string &shader);

    void mouseMoved(const Event<MouseMoved> & /*e*/);
    void keyPressed(const Event<KeyPressed> & /*e*/);

    void onWindowResized(const Event<WindowResized> & /*ev*/);
};

void
Anim::link(const Event<InitEvent> &e)
{
    engine.events().animate.reg(*this, &Anim::animate);
    engine.events().render.reg(*this, &Anim::renderScene);
    GameWindow &win = engine.window();
    win.events().mouseMoved.reg(*this, &Anim::mouseMoved);
    win.events().windowResized.reg(*this, &Anim::onWindowResized);
    engine.keyHandler().keyPressedEvent().reg(*this, &Anim::keyPressed);
    e.info.success = true;
}

void
Anim::init(const Event<InitEvent> &ev)
{
    loadResources(sys::fs::join(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR), "data"));
    engine.out() << "in init()" << sys::io::endl;

    mouse_look.camera(&camera);
    engine.enablePlugin(camera);
    engine.enablePlugin(mouse_look);

    fpsTimer = std::make_shared<Timer>(engine);
    fpsTimer->start(1.f, true);

    engine.gameLoop().ticks(100);
    engine.gameLoop().syncDraw(true);

    use_spotlight = false;
    spotlight_smooth = false;

    wireframe_mode = false;
    // GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, GL_LINE);

    shade_mode = SHADE_MODE_DEFAULT;

    light_rotation_speed = 0.0025f;
    light_angular_position = 0.f;

    gamma_correction = 1.35f;

#ifdef MESH_MESH
    cubeModel.primType(GL_QUADS);
    cubeModel.drawType(glt::DrawArrays);
#endif
    glt::primitives::unitCube3(cubeModel);
    cubeModel.send();

    glt::primitives::sphere(sphereModel, 1.f, 26, 13);
    sphereModel.send();

    {
#ifdef MESH_MESH
        groundModel.primType(GL_QUADS);
#endif

        Vertex v{};
        v.normal = vec3(0.f, 1.f, 0.f);
        v.position = vec3(0.f, 0.f, 0.f);
        groundModel.addVertex(v);
        v.position = vec3(1.f, 0.f, 0.f);
        groundModel.addVertex(v);
        v.position = vec3(1.f, 0.f, 1.f);
        groundModel.addVertex(v);
        v.position = vec3(0.f, 0.f, 1.f);
        groundModel.addVertex(v);

        groundModel.send();
    }

    {
        ScreenVertex v{};
        v.normal = vec3(0.f, 0.f, 1.f);
        v.position = vec3(0, 0, 0);
        screenQuad.add(v);
        v.position = vec3(1, 0, 0);
        screenQuad.add(v);
        v.position = vec3(1, 1, 0);
        screenQuad.add(v);
        v.position = vec3(0, 1, 0);
        screenQuad.add(v);

        screenQuad.send();
    }

    camera.frame().origin = vec3(6.36_r, 5.87_r, 1.97_r);
    camera.frame().setXZ(normalize(vec3(-0.29_r, 0_r, 0.95_r)),
                         normalize(vec3(-0.8_r, -0.54_r, -0.25_r)));

    teapot1.frame.setXZ(vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
    teapot1.frame.origin = vec3(5.f, 4.f, 4.f);

    teapot1.material.ambientContribution = 0.4f;
    teapot1.material.diffuseContribution = 0.6f;
    teapot1.material.specularContribution = 0.2f;
    teapot1.material.shininess = 110;
    teapot1.material.glow = 0;
    teapot1.color = glt::color(0xFF, 0xFF, 0xFF);

    teapot2.frame.setXZ(vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
    teapot2.frame.origin = vec3(3.f, 4.f, 12.f);
    teapot2.frame.rotateLocal(degToRad(110.f), vec3(0.f, 0.f, 1.f));
    teapot2.material.ambientContribution = 0.4f;
    teapot2.material.diffuseContribution = 0.6f;
    teapot2.material.specularContribution = 0.15f;
    teapot2.material.shininess = 35;
    teapot2.material.glow = 0.2_r;
    teapot2.color = glt::color(0xFF, 0x8C, 0x00);

    {
        auto w = engine.window().windowWidth();
        auto h = engine.window().windowHeight();
        glt::TextureRenderTarget::Params ps;
        ps.samples = 4;
        ps.buffers = glt::RT_COLOR_BUFFER | glt::RT_DEPTH_BUFFER;
        tex_render_target =
          std::make_shared<glt::TextureRenderTarget>(w, h, ps);
        engine.renderManager().setDefaultRenderTarget(tex_render_target.get());
    }

    {
        size_t w = 512;
        size_t h = 512;
        glt::TextureRenderTarget::Params ps;
        ps.buffers = glt::RT_COLOR_BUFFER;
        ps.filter_mode = glt::TextureSampler::FilterLinear;
        glow_render_target_src =
          std::make_shared<glt::TextureRenderTarget>(w, h, ps);
        glow_render_target_dst =
          std::make_shared<glt::TextureRenderTarget>(w, h, ps);
    }

    {
        const auto N = KERNEL_SIZE;
        const auto N2 = float(N - 1) * 0.5f;
        for (size_t i = 0; i < glow_kernel.size(); ++i) {
            float x = float(i) - N2;

            const float SIG = 0.84089642_r;
            const float SIG2 = SIG * SIG;
            const float SQRT_PI = 1.7724538509055_r;
            const float SQRT_2 = 1.4142135623730_r;
            glow_kernel[i] = 1.f / (SQRT_PI * SQRT_2 * SIG) *
                             math::exp(-1 / (2 * SIG2) * (x * x));
        }
    }

    ev.info.success = true;
}

void
Anim::animate(const Event<AnimationEvent> & /*unused*/)
{
    light_angular_position =
      wrapPi(light_angular_position + light_rotation_speed);
}

vec3_t
Anim::lightPosition(real interpolation)
{
    real theta =
      wrapPi(light_angular_position + interpolation * light_rotation_speed);
    vec3_t d = vec3(math::cos(theta), 0.f, math::sin(theta));
    return d * LIGHT_ROTATION_RAD + LIGHT_CENTER_OF_ROTATION;
}

void
Anim::renderScene(const Event<RenderEvent> &e)
{
    real interpolation = e.info.interpolation;
    glt::RenderManager &rm = engine.renderManager();

    glt::RenderTarget *render_target;
    real bg_alpha;
#ifdef RENDER_NOGLOW
    render_target = &engine.window().renderTarget();
    bg_alpha = 1.f;
#else
    render_target = tex_render_target.get();
    bg_alpha = 0.f;
#endif

    rm.setActiveRenderTarget(render_target);
    rm.activeRenderTarget()->clearColor(
      glt::color(vec4(vec3(0.65f), bg_alpha)));
    rm.activeRenderTarget()->clear();

    GL_CALL(glDisable, GL_BLEND);
    GL_CALL(glEnable, GL_DEPTH_TEST);
    GL_CALL(glDisable, GL_ALPHA_TEST);

    light = lightPosition(interpolation);

    ecLight = rm.geometryTransform().transformPoint(light);

    {
        vec3_t spot_center = vec3(5.f, 2.f, 8.f);
        vec3_t spotdir = spot_center - light;
        spotdir = normalize(spotdir);
        ec_spotlight_dir = rm.geometryTransform().normalMatrix() * spotdir;
        ec_spotlight_dir = normalize(ec_spotlight_dir);
    }

    renderLight();

    renderTable("teapotTextured");

    // { // render a shadow of the table
    //     glt::GeometryTransform& gt = rm.geometryTransform();
    //     glt::SavePoint sp(rm.geometryTransform().save());

    //     const point3_t& s = light;

    //     mat4_t shadowProjection = mat4(vec4(-s.y, 0.f,  0.f,  0.f),
    //                                vec4( s.x, 0.f,  s.z,  1.f),
    //                                vec4( 0.f, 0.f, -s.y,  0.f),
    //                                vec4( 0.f, 0.f,  0.f, -s.y));

    //     gt.concat(shadowProjection);

    //     renderTable(teapotShader);
    // }

    //    renderGround();
    renderTeapot(teapot1);
    renderTeapot(teapot2);

#ifdef RENDER_GLOW
    GL_CALL(glDisable, GL_DEPTH_TEST);

    { // create the glow texture
        engine.renderManager().setActiveRenderTarget(
          glow_render_target_src.get());
        auto glow_pass0 = engine.shaderManager().program("glow_pass0");
        ASSERT(glow_pass0);
        glow_pass0->use();
        tex_render_target->sampler().bind(0);
        glt::Uniforms(*glow_pass0)
          .optional("texture0", glt::Sampler(tex_render_target->sampler(), 0));
        screenQuad.draw();
        tex_render_target->sampler().unbind(0);
    }

    // blur the glow texture
    auto from = glow_render_target_src;
    auto to = glow_render_target_dst;
    auto glow_pass1 = engine.shaderManager().program("glow_pass1");

    mat2_t texMat0 = mat2();
    mat2_t texMat1 = mat2(vec2(0, 1), vec2(1, 0));

    ASSERT(glow_pass1);
    glow_pass1->use();

    ArrayView<const float> kernel = { glow_kernel.size(), &glow_kernel[0] };
    for (int pass = 0; pass < 3; ++pass) {
        engine.renderManager().setActiveRenderTarget(to.get());
        from->sampler().bind(0);
        glt::Uniforms(*glow_pass1)
          .mandatory("texture0", glt::Sampler(from->sampler(), 0))
          .optional("kernel", kernel)
          .optional("texMat", texMat0);
        screenQuad.draw();
        from->sampler().unbind(0);

        engine.renderManager().setActiveRenderTarget(from.get());
        to->sampler().bind(0);
        glt::Uniforms(*glow_pass1)
          .mandatory("texture0", glt::Sampler(to->sampler(), 0))
          .optional("kernel", kernel)
          .optional("texMat", texMat1);
        screenQuad.draw();
        to->sampler().unbind(0);
    }

    { // render texture to window framebuffer
        engine.renderManager().setActiveRenderTarget(
          &engine.window().renderTarget());

        auto postprocShader = engine.shaderManager().program("postproc");
        ASSERT(postprocShader);
        postprocShader->use();

        tex_render_target->sampler().bind(0);
        glow_render_target_dst->sampler().bind(1);
        glt::Uniforms(*postprocShader)
          .optional("texture0", glt::Sampler(tex_render_target->sampler(), 0))
          .optional("texture1", glt::Sampler(from->sampler(), 1));
        screenQuad.draw();
        tex_render_target->sampler().unbind(0);
        glow_render_target_dst->sampler().unbind(1);
    }
#endif

    if (fpsTimer->fire()) {
#define INV(x) (((x) * (x)) <= 0 ? -1 : 1.0 / (x))
        glt::FrameStatistics fs = engine.renderManager().frameStatistics();
        double fps = INV(fs.avg);
        double min = INV(fs.max);
        double max = INV(fs.min);
        double avg = INV(fs.avg);
        sys::io::stderr() << "Timings (FPS/Render Avg/Render Min/Render Max): "
                          << fps << "; " << avg << "; " << min << "; " << max
                          << sys::io::endl;
    }
}

void
Anim::onWindowResized(const Event<WindowResized> &ev)
{
    auto w = ev.info.window.windowWidth();
    auto h = ev.info.window.windowHeight();
    engine.renderManager().setActiveRenderTarget(nullptr);
    tex_render_target->resize(w, h);
    engine.renderManager().setDefaultRenderTarget(tex_render_target.get());
}

void
Anim::setupTeapotShader(const std::string &progname,
                        const vec4_t &surfaceColor,
                        const MaterialProperties &mat)
{
    glt::RenderManager &rm = engine.renderManager();
    auto prog = engine.shaderManager().program(progname);
    if (!prog) {
        ASSERT(prog, "undefined program: " + progname);
        return;
    }

    ASSERT(!prog->wasError());
    ASSERT(prog->validate());

    prog->use();

    vec4_t materialSelector = vec4((shade_mode & SHADE_MODE_AMBIENT) != 0,
                                   (shade_mode & SHADE_MODE_DIFFUSE) != 0,
                                   (shade_mode & SHADE_MODE_SPECULAR) != 0,
                                   1.f);

    real scale = 1 / (2.f * math::PI);
    real specular_factor =
      mat.specularContribution * (mat.shininess + 2) * scale;
    vec4_t vm = vec4(mat.ambientContribution,
                     mat.diffuseContribution,
                     specular_factor,
                     mat.shininess);

    glt::Uniforms us(*prog);
    us.optional("projectionMatrix", rm.geometryTransform().projectionMatrix());
    us.optional("mvMatrix", rm.geometryTransform().mvMatrix());
    us.optional("normalMatrix", rm.geometryTransform().normalMatrix());
    us.optional("surfaceColor", surfaceColor);
    us.optional("materialProperties", vm * materialSelector);
    us.optional("ecLight", ecLight);
    us.optional("gammaCorrection", gamma_correction);
    us.optional("spotDirection", ec_spotlight_dir);
    us.optional("useSpot", 1.f * use_spotlight);
    us.optional("spotSmooth", 1.f * spotlight_smooth);
    us.optional("texData", glt::BoundTexture(GL_SAMPLER_2D, 0));
#ifdef RENDER_NOGLOW
    us.optional("glow", real(1));
#else
    us.optional("glow", mat.glow /* real(0.8) */);
#endif
}

void
Anim::renderLight()
{
    glt::RenderManager &rm = engine.renderManager();
    glt::SavePoint sp(rm.geometryTransform().save());

    rm.geometryTransform().translate(light);
    rm.geometryTransform().scale(vec3(0.66f));

    MaterialProperties mat = { 0.8f, 0.2f, 1.f, 120.f, 0.8f };

    setupTeapotShader("teapot", vec4(1.f, 1.f, 0.f, 1.f), mat);
    sphereModel.draw();
}

void
Anim::renderTeapot(const Teapot &teapot)
{
    glt::RenderManager &rm = engine.renderManager();
    glt::SavePoint sp(rm.geometryTransform().save());

    rm.geometryTransform().concat(transformationLocalToWorld(teapot.frame));
    rm.geometryTransform().scale(vec3(13.f));

    setupTeapotShader("teapot", teapot.color.vec4(), teapot.material);
    teapotModel.draw();
}

void
Anim::renderGround()
{
    glt::RenderManager &rm = engine.renderManager();
    glt::SavePoint sp(rm.geometryTransform().save());

    rm.geometryTransform().scale(vec3(50.f));
    rm.geometryTransform().translate(vec3(-0.5f, 0.f, -0.5f));

    MaterialProperties mat = { 0.f, 1.f, 0.f, 30.f, 0 };
    vec4_t color = glt::color(0xcd, 0xc9, 0xc9).vec4();
    setupTeapotShader("teapot", color, mat);
    groundModel.draw();
}

void
Anim::renderTable(const std::string &shader)
{
    glt::RenderManager &rm = engine.renderManager();
    glt::GeometryTransform &gt = rm.geometryTransform();
    glt::SavePoint sp(gt.save());

    gt.scale(vec3(10.f, 4.f, 16.f));

    gt.dup();

    woodTexture.bind(0);

    vec4_t color = glt::color(0xcd, 0x85, 0x3f).vec4();
    MaterialProperties mat{};
    mat.ambientContribution = 0.55f;
    mat.diffuseContribution = 0.6f;
    mat.specularContribution = 0.075f;
    mat.shininess = 30;

    real table_height = 1.f;
    real table_thickness = 1 / 33.f;
    real foot_height = table_height - table_thickness;
    real foot_width = 0.03f;
    real foot_depth = 10.f / 16.f * foot_width;
    real foot_x_dist = 1.f - foot_width;
    real foot_z_dist = 1.f - foot_depth;

    vec3_t foot_dim = vec3(foot_width, foot_height, foot_depth);

    gt.translate(vec3(0.f, foot_height, 0.f));
    gt.scale(vec3(1.f, table_thickness, 1.f));
    setupTeapotShader(shader, color, mat);
    cubeModel.draw();

    for (uint32_t x = 0; x < 2; ++x) {
        for (uint32_t z = 0; z < 2; ++z) {
            gt.pop();
            gt.dup();
            gt.translate(vec3(x * foot_x_dist, 0.f, z * foot_z_dist));
            gt.scale(foot_dim);
            setupTeapotShader(shader, color, mat);
            cubeModel.draw();
        }
    }

    woodTexture.unbind(0);
}

void
Anim::mouseMoved(const Event<MouseMoved> &e)
{
    auto dx = e.info.dx;
    auto dy = e.info.dy;

    dx = -dx;

    if (engine.keyHandler().keyState(ge::KeyCode::M) <= KeyState::Pressed) {
        teapot1.frame.rotateWorld(-dx * 0.001f, camera.frame().localY());
        teapot1.frame.rotateWorld(dy * 0.001f, camera.frame().localX());
        e.abort = true;
    }
}

void
Anim::keyPressed(const Event<KeyPressed> &e)
{
    using K = ge::KeyCode;

    BEGIN_NO_WARN_SWITCH
    switch (e.info.key.value) {
    case K::F:
        wireframe_mode = !wireframe_mode;
        GL_CALL(
          glPolygonMode, GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);
        break;
    case K::H:

        if (shade_mode == SHADE_MODE_DEFAULT)
            shade_mode = 0;
        else
            shade_mode = SHADE_MODE_DEFAULT;
        break;
    case K::J:
        shade_mode ^= SHADE_MODE_AMBIENT;
        break;
    case K::K:
        shade_mode ^= SHADE_MODE_DIFFUSE;
        break;
    case K::L:
        shade_mode ^= SHADE_MODE_SPECULAR;
        break;
    case K::O:
        gamma_correction += 0.05f;
        break;
    case K::P:
        gamma_correction -= 0.05f;
        break;
    case K::G:
        if (use_spotlight && spotlight_smooth) {
            use_spotlight = false;
            spotlight_smooth = false;
        } else if (use_spotlight) {
            spotlight_smooth = true;
        } else {
            use_spotlight = true;
        }
        break;
    }
    END_NO_WARN_SWITCH
}

struct FileDeleter
{
    void operator()(FILE *h) noexcept { fclose(h); }
};

using unique_file = std::unique_ptr<FILE, FileDeleter>;

void
Anim::loadResources(const std::string &dir)
{
    data_dir = dir;

    int w, h;
    uint32_t *wood_data;
    auto file = unique_file(fopen((data_dir + "/wood.bmp").c_str(), "rb"));
    if (!file) {
        ERR("couldnt open data/wood.bmp");
        return;
    }
    if (bmp_read(file.get(), &w, &h, &wood_data) != BMP_OK) {
        ERR("couldnt load data/wood.bmp");
        return;
    }

    GL_CALL(glTextureImage2DEXT,
            *woodTexture.data()->ensureHandle(),
            GL_TEXTURE_2D,
            0,
            GL_RGB8,
            GLint(w),
            GLint(h),
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            wood_data);
    free(wood_data);
    woodTexture.filterMode(glt::TextureSampler::FilterLinear);

    auto nfaces = parse_sply((data_dir + "/teapot.sply").c_str(), teapotModel);
    if (nfaces < 0) {
        ERR("couldnt parse teapot model");
        return;
    }
    sys::io::stdout() << "parsed teapot model: " << nfaces << " vertices"
                      << sys::io::endl;

#ifdef MESH_MESH
    teapotModel.primType(GL_QUADS);
#endif
    teapotModel.drawType(glt::DrawElements);
    teapotModel.send();
}

int
main(int argc, char *argv[])
{
    EngineOptions opts;
    Engine engine;
    engine.setDevelDataDir(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR));
    Anim anim(engine);
    opts.parse(&argc, &argv);
    opts.inits.init.reg(anim, &Anim::init);
    opts.inits.init.reg(anim, &Anim::link);
    return engine.run(opts);
}

int32_t
parse_sply(const char *filename, CubeMeshOf<Vertex> &model)
{
    auto data = unique_file(fopen(filename, "rb"));
    if (!data)
        return -1;

    char line[512];

    std::vector<Vertex> verts;
    uint32_t nverts = 0;
    uint32_t nfaces = 0;

    if (fscanf(data.get(), "%u\n%u\n", &nverts, &nfaces) != 2)
        return -1;

    while (fgets(line, sizeof line, data.get()) != nullptr &&
           verts.size() < nverts) {
        point3_t p;
        direction3_t n;
        int nparsed = sscanf(line,
                             "%" R_FMT " %" R_FMT " %" R_FMT " %" R_FMT
                             " %" R_FMT " %" R_FMT,
                             &p[0],
                             &p[1],
                             &p[2],
                             &n[0],
                             &n[1],
                             &n[2]);

        if (nparsed != 6)
            return -1;

        Vertex v{};
        v.position = p;
        v.normal = normalize(n);
        verts.push_back(v);
    }

    if (verts.size() != nverts)
        return -1;

    uint32_t faces = 0;

    for (const auto &vert : verts)
        model.addVertex(vert);

    while (fgets(line, sizeof line, data.get()) != nullptr && faces < nfaces) {
        uint32_t n, i, j, k, l;
        int nparsed = sscanf(line, "%u %u %u %u %u", &n, &i, &j, &k, &l);

        if (nparsed != 5 || n != 4)
            return -1;

#ifdef MESH_CUBEMESH

        model.pushElement(i);
        model.pushElement(j);
        model.pushElement(k);
        model.pushElement(i);
        model.pushElement(k);
        model.pushElement(l);

#else

        model.pushElement(i);
        model.pushElement(j);
        model.pushElement(k);
        model.pushElement(l);

#endif

        ++faces;
    }

    return int32_t(nfaces) * 4;

    //    return faces == nfaces ? int32(nfaces) * 4: -1;
}
