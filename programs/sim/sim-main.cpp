#include "sim.hpp"

#include "ge/Camera.hpp"
#include "ge/Command.hpp"
#include "ge/Engine.hpp"
#include "ge/GameWindow.hpp"
#include "ge/MouseLookPlugin.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/Mesh.hpp"
#include "glt/TextureRenderTarget.hpp"
#include "glt/primitives.hpp"
#include "glt/utils.hpp"
#include "math/ivec3.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/real.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

using namespace math;

inline constexpr real SPHERE_DENSITY = 999_r;

// inline constexpr vec4_t SPHERE_COLOR = vec4(0.f, 0.f, 1.f, 1.f);

inline constexpr vec3_t LIGHT_POS = vec3(11.f, 18.f, 11.f);

// inline constexpr real FPS_RENDER_CYCLE = 1_r;

inline constexpr float GAMMA = 1.8f;

inline constexpr glt::color CONNECTION_COLOR(0x00, 0xFF, 0x00);

inline constexpr size_t AA_SAMPLES = 4;

inline constexpr size_t SPHERE_LOD_MAX = 6;

struct SphereLOD
{
    size_t level;
};

#define SPHERE_INSTANCED_TEXTURED
// (doesnt work atm)
// #define SPHERE_INSTANCED_ARRAY

#ifdef SPHERE_INSTANCED_TEXTURED

// dont change layout (directly mapped to texture)
HU_BEGIN_PACKED
struct SphereInstance
{
    vec3_t pos;
    float rad;
    vec3_t col_rgb;
    float shininess;
} HU_PACKED;
HU_END_PACKED
#elif defined(SPHERE_INSTANCED_ARRAY)
HU_BEGIN_PACKED
struct SphereInstance
{
    mat4_t mvMatrix;
    vec4_t colorShininess;
} HU_PACKED;
HU_END_PACKED
#else
#error "no SPHERE_INSTANCED_* specified"
#endif

DEF_GL_MAPPED_TYPE(Vertex, (vec4_t, position), (vec3_t, normal))

namespace {

using Mesh = glt::Mesh<Vertex>;
using CubeMesh = glt::CubeMesh<Vertex>;

} // namespace

struct Game
{

    ge::Camera camera;
    ge::MouseLookPlugin mouse_look;

    CubeMesh wallBatch;
    Mesh sphereBatches[SPHERE_LOD_MAX];
    Mesh lineBatch;
    CubeMesh rectBatch;

    World world;
    float sphere_speed{};
    Sphere sphere_proto{};
    Renderer renderer;

    struct
    {
        vec3_t ecLightPos;
        vec3_t ecSpotDir;
        float spotAngle;
    } wallUniforms{};

    struct
    {
        vec3_t ecLightPos;
        vec3_t ecSpotDir;
        float spotAngle;
    } sphereUniforms{};

    glt::TextureRenderTarget *textureRenderTarget{};
    ge::Engine *engine{};

    float game_speed{};

    bool use_interpolation{};
    bool render_spheres_instanced{};
    bool indirect_rendering{};

    std::vector<SphereInstance> sphere_instances[SPHERE_LOD_MAX];

    Game();

    ~Game()
    {
        if (engine != nullptr)
            engine->renderManager().shutdown();
        delete textureRenderTarget;
    }

    void updateIndirectRendering(bool indirect);
    void resizeRenderTargets();

    void init(const ge::Event<ge::InitEvent> & /*ev*/);
    void link(ge::Engine &e);

    void constrainCameraMovement(const ge::Event<ge::CameraMoved> & /*ev*/);
    void animate(const ge::Event<ge::AnimationEvent> & /*ev*/);
    void renderScene(const ge::Event<ge::RenderEvent> & /*ev*/);
    void renderWorld(float dt);

    void windowResized(const ge::Event<ge::WindowResized> & /*ev*/);
    void handleInternalEvents();
    void spawn_sphere();

    SphereLOD calc_sphere_lod(const Sphere &s);
    void render_sphere(const Sphere &s, const SphereModel &m);
    void end_render_spheres();
    void render_box(const glt::AABB &box);
    void render_con(const point3_t &a, const point3_t &b);
    void render_hud();

    void update_sphere_mass();
};

Game::Game() : renderer(*this) {}

void
Game::init(const ge::Event<ge::InitEvent> &ev)
{
    ge::Engine &e = ev.info.engine;
    engine = &e;

    link(e);

    textureRenderTarget = nullptr;
    indirect_rendering = true;
    updateIndirectRendering(indirect_rendering);
    render_spheres_instanced = true;

    {
        Vertex v{};
        v.normal = vec3(0.f, 0.f, 1.f);
        v.position = vec4(-1.f, -1.f, 0.f, 1.f);
        rectBatch.add(v);
        v.position = vec4(1.f, -1.f, 0.f, 1.f);
        rectBatch.add(v);
        v.position = vec4(1.f, 1.f, 0.f, 1.f);
        rectBatch.add(v);
        v.position = vec4(-1.f, 1.f, 0.f, 1.f);
        rectBatch.add(v);

        rectBatch.send();
    }

    e.gameLoop().ticks(200);
    e.gameLoop().syncDraw(false);

    use_interpolation = true;

    if (!world.init())
        return;

    world.render_by_distance = true;
    ;

    sphere_speed = 10.f;
    sphere_proto.state = Bouncing;
    sphere_proto.m = 1.f;
    sphere_proto.r = 0.3f;

    game_speed = 1.f;

    glt::primitives::unitCubeEverted(wallBatch);
    wallBatch.send();

    struct
    {
        int32_t a, b;
    } sphere_params[SPHERE_LOD_MAX] = { { 12, 6 },  { 16, 8 },  { 20, 10 },
                                        { 24, 12 }, { 26, 13 }, { 36, 18 } };

    for (size_t i = 0; i < SPHERE_LOD_MAX; ++i) {
        glt::primitives::sphere(
          sphereBatches[i], 1.f, sphere_params[i].a, sphere_params[i].b);
        sphereBatches[i].send();
    }

    camera.frame().origin = vec3(0.f, 0.f, 0.f);

    update_sphere_mass();

    Vertex vert{};
    vert.normal = vec3(1.f, 0.f, 0.f);
    vert.position = vec4(vec3(0.f), 1.f);
    lineBatch.addVertex(vert);
    vert.position = vec4(1.f, 0.f, 0.f, 1.f);
    lineBatch.addVertex(vert);
    lineBatch.send();

    ev.info.success = true;
}

void
Game::constrainCameraMovement(const ge::Event<ge::CameraMoved> &ev)
{
    ev.info.allowed_step = ev.info.step;
    if (!world.canMoveCamera(camera.frame().origin, ev.info.allowed_step))
        ev.info.allowed_step = vec3(0.f);
}

void
Game::animate(const ge::Event<ge::AnimationEvent> &ev)
{
    auto dt = game_speed * ev.info.engine.gameLoop().tickDuration();
    world.simulate(real(dt));
}

void
Game::windowResized(const ge::Event<ge::WindowResized> &ev)
{
    auto width = ev.info.width;
    auto height = ev.info.height;

    // ge::Engine& e = ev.info.engine;

    // ASSERT(e.renderManager().renderTarget().viewport() == glt::Viewport());
    // ASSERT(e.renderManager().renderTarget().width() == width);
    // ASSERT(e.renderManager().renderTarget().height() == height);

    sys::io::stderr() << "new window dimensions: " << width << "x" << height
                      << sys::io::endl;

    resizeRenderTargets();
}

void
Game::update_sphere_mass()
{
    float V = 4.f / 3.f * math::PI * math::cubed(sphere_proto.r);
    sphere_proto.m = V * SPHERE_DENSITY;
}

static float
rand1()
{
    return rand() * (1.f / RAND_MAX);
}

static glt::color
randomColor()
{
    return glt::color(
      uint8_t(rand1() * 255), uint8_t(rand1() * 255), uint8_t(rand1() * 255));
}

void
Game::spawn_sphere()
{
    vec3_t direction = -camera.frame().localZ();
    sphere_proto.center =
      camera.frame().origin + direction * (sphere_proto.r + 1.1f);
    sphere_proto.v = direction * sphere_speed;

    SphereModel model;
    model.color = randomColor();
    model.shininess = 10.f + rand1() * 60;

    world.spawnSphere(sphere_proto, model);
}

void
Game::renderScene(const ge::Event<ge::RenderEvent> &ev)
{
    float interpolation = ev.info.interpolation;
    ge::Engine &e = ev.info.engine;

    engine = &e;

    glt::RenderManager &renderManager = e.renderManager();

    if (!use_interpolation)
        interpolation = 0.f;

    renderManager.activeRenderTarget()->clear(glt::RT_DEPTH_BUFFER);
    GL_CALL(glEnable, GL_DEPTH_TEST);

    //    e.window().window().setActive();

    auto dt = real(interpolation * game_speed * e.gameLoop().tickDuration());

    renderWorld(dt);

    if (indirect_rendering) {
        GL_CALL(glDisable, GL_DEPTH_TEST);

        renderManager.setActiveRenderTarget(&e.window().renderTarget());
        auto postprocShader = e.shaderManager().program("postproc");
        ASSERT(postprocShader);
        postprocShader->use();

        textureRenderTarget->sampler().bind(0);
        glt::Uniforms(*postprocShader)
          .optional("gammaCorrection", GAMMA)
          .mandatory("textures",
                     glt::Sampler(textureRenderTarget->sampler(), 0));
        rectBatch.draw();
        textureRenderTarget->sampler().unbind(0);
    }

    render_hud();
}

void
Game::renderWorld(float dt)
{
    glt::GeometryTransform &gt = engine->renderManager().geometryTransform();
    const vec3_t eyeLightPos = gt.transformPoint(LIGHT_POS);
    const vec3_t spotDirection = normalize(vec3(0.f) - LIGHT_POS);
    const vec3_t ecSpotDirection = gt.transformVector(spotDirection);

    wallUniforms.ecLightPos = eyeLightPos;
    wallUniforms.ecSpotDir = ecSpotDirection;
    wallUniforms.spotAngle = 0.91f;

    sphereUniforms.ecLightPos = eyeLightPos;
    sphereUniforms.ecSpotDir = ecSpotDirection;
    sphereUniforms.spotAngle = 0.91f;

    world.render(renderer, dt);
}

const glt::ViewFrustum &
Renderer::frustum()
{
    return game.engine->renderManager().viewFrustum();
}

const glt::Frame &
Renderer::camera()
{
    return game.camera.frame();
}

void
Game::updateIndirectRendering(bool indirect)
{
    glt::RenderTarget *rt;

    if (indirect) {

        if (textureRenderTarget == nullptr) {
            auto w = engine->window().windowWidth();
            auto h = engine->window().windowHeight();
            glt::TextureRenderTarget::Params ps;
            ps.buffers = glt::RT_COLOR_BUFFER | glt::RT_DEPTH_BUFFER;
            textureRenderTarget = new glt::TextureRenderTarget(w, h, ps);
        }

        rt = textureRenderTarget;
    } else {
        rt = &engine->window().renderTarget();
    }

    indirect_rendering = indirect;

    engine->renderManager().setActiveRenderTarget(nullptr);
    resizeRenderTargets();
    engine->renderManager().setDefaultRenderTarget(rt);
}

void
Game::resizeRenderTargets()
{
    auto width = engine->window().windowWidth();
    auto height = engine->window().windowHeight();

    if (indirect_rendering) {

        auto stride = 1u;
        while (stride * stride < AA_SAMPLES)
            ++stride;

        textureRenderTarget->resize(width * stride, height * stride);
    }
}

void
Renderer::renderSphere(const Sphere &s, const SphereModel &m)
{
    game.render_sphere(s, m);
}

void
Renderer::endRenderSpheres()
{
    game.end_render_spheres();
}

void
Game::end_render_spheres()
{
    if (render_spheres_instanced) {

        for (size_t lod = 0; lod < SPHERE_LOD_MAX; ++lod) {
            auto num = sphere_instances[lod].size();

            if (num == 0)
                continue;

#ifdef SPHERE_INSTANCED_TEXTURED
            glt::TextureSampler sphereMap(
              std::make_shared<glt::TextureData>(glt::Texture1D));

            sphereMap.filterMode(glt::TextureSampler::FilterNearest);
            sphereMap.clampMode(glt::TextureSampler::ClampRepeat);
            sphereMap.bind(0);
            GL_CALL(glTexImage1D,
                    GL_TEXTURE_1D,
                    0,
                    GL_RGBA32F,
                    GLsizei(num * 2),
                    0,
                    GL_RGBA,
                    GL_FLOAT,
                    &sphere_instances[lod][0]);

            sphere_instances[lod].clear();

            auto sphereInstancedShader =
              engine->shaderManager().program("sphereInstanced");
            ASSERT(sphereInstancedShader);
            sphereInstancedShader->use();

            glt::GeometryTransform &gt =
              engine->renderManager().geometryTransform();
            glt::Uniforms(*sphereInstancedShader)
              .optional("normalMatrix", gt.normalMatrix())
              .optional("vMatrix", gt.mvMatrix())
              .optional("pMatrix", gt.projectionMatrix())
              .optional("ecLight", sphereUniforms.ecLightPos)
              .optional("gammaCorrection", indirect_rendering ? 1.f : GAMMA)
              .mandatory("instanceData", glt::Sampler(sphereMap, 0));

            sphereBatches[lod].drawInstanced(num);
            sphereMap.unbind(0);
            sphereMap.free();

#elif defined(SPHERE_INSTANCED_ARRAY)

            auto sphereInstanced2Shader =
              engine->shaderManager().program("sphereInstanced2");
            ASSERT(sphereInstanced2Shader);
            sphereInstanced2Shader->use();

            GLBufferObject per_instance_vbo;
            per_instance_vbo.ensure();
            GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, *per_instance_vbo);
            GL_CALL(glBufferData,
                    GL_ARRAY_BUFFER,
                    sizeof(SphereInstance) * num,
                    &sphere_instances[lod][0],
                    GL_STREAM_DRAW);

            sphereBatches[lod].bind();
            GLint attr_mvMatrix;
            GL_ASSIGN_CALL(attr_mvMatrix,
                           glGetAttribLocation,
                           sphereInstanced2Shader->program(),
                           "mvMatrix");
            ASSERT(attr_mvMatrix >= 0);
            GLint attr_colorShininess;
            GL_ASSIGN_CALL(attr_colorShininess,
                           glGetAttribLocation,
                           sphereInstanced2Shader->program(),
                           "colorShininess");
            ASSERT(attr_colorShininess >= 0);

            for (uint32 col = 0; col < 4; ++col) {
                GL_CALL(glVertexAttribPointer,
                        attr_mvMatrix + col,
                        4,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(vec4_t),
                        (void *) (offsetof(SphereInstance, mvMatrix) +
                                  col * sizeof(vec4_t)));
                GL_CALL(glVertexAttribDivisor, attr_mvMatrix + col, 1);
                GL_CALL(glEnableVertexAttribArray, attr_mvMatrix + col);
            }

            GL_CALL(glVertexAttribPointer,
                    attr_colorShininess,
                    4,
                    GL_FLOAT,
                    GL_FALSE,
                    sizeof(SphereInstance),
                    (void *) offsetof(SphereInstance, colorShininess));
            GL_CALL(glVertexAttribDivisor, attr_colorShininess, 1);
            GL_CALL(glEnableVertexAttribArray, attr_colorShininess);

            GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

            glt::GeometryTransform &gt =
              engine->renderManager().geometryTransform();
            glt::Uniforms(*sphereInstanced2Shader)
              .optional("pMatrix", gt.projectionMatrix())
              .optional("ecLight", sphereUniforms.ecLightPos)
              .optional("gammaCorrection", indirect_rendering ? 1.f : GAMMA);

            sphereBatches[lod].drawInstanced(num);
            sphereBatches[lod].bind();

            for (uint32 col = 0; col < 4; ++col) {
                GL_CALL(glDisableVertexAttribArray, attr_mvMatrix + col);
            }

            GL_CALL(glDisableVertexAttribArray, attr_colorShininess);
            GL_CALL(glBindVertexArray, 0);
#endif
        }
    }
}

SphereLOD
Game::calc_sphere_lod(const Sphere &s)
{
    glt::GeometryTransform &gt = engine->renderManager().geometryTransform();
    vec3_t ecCoord = gt.transformPoint(s.center);

    // near upper right corner of frustm (view coord)
    vec4_t nur_corner =
      gt.inverseProjectionMatrix() * vec4(-1.f, -1.f, -1.f, 1.f);

    float x_max = math::abs(nur_corner[0]);
    float y_max = math::abs(nur_corner[1]);
    float z_min = math::abs(nur_corner[2]);

    float size = min(x_max, y_max);

    // calculate lod, use projected radius on screen
    float r_proj = s.r / ecCoord[2] * z_min;
    float screen_rad = r_proj / size;
    if (screen_rad < 0)
        screen_rad = 0;

    screen_rad *= 2;

    SphereLOD lod{};
    lod.level = size_t(screen_rad * SPHERE_LOD_MAX);
    if (lod.level >= SPHERE_LOD_MAX)
        lod.level = SPHERE_LOD_MAX - 1;

    ASSERT(lod.level < SPHERE_LOD_MAX);

    return lod;
}

void
Game::render_sphere(const Sphere &s, const SphereModel &m)
{

    SphereLOD lod = calc_sphere_lod(s);

    if (render_spheres_instanced) {

#ifdef SPHERE_INSTANCED_TEXTURED

        SphereInstance inst{};
        inst.pos = s.center;
        inst.rad = s.r;
        inst.col_rgb = vec3(m.color.vec4());
        inst.shininess = m.shininess;

#elif defined(SPHERE_INSTANCED_ARRAY)

        SphereInstance inst;
        inst.mvMatrix = engine->renderManager().geometryTransform().mvMatrix();
        inst.colorShininess = vec4(vec3(m.color.vec4()), m.shininess);

#endif
        DEBUG_ASSERT(lod.level < SPHERE_LOD_MAX);
        sphere_instances[lod.level].push_back(inst);

    } else {
        glt::GeometryTransform &gt =
          engine->renderManager().geometryTransform();
        const vec3_t &pos = s.center;
        glt::SavePoint sp(gt.save());
        gt.translate(pos);
        gt.scale(vec3(s.r));

        auto sphereShader = engine->shaderManager().program("sphere");
        ASSERT(sphereShader);

        sphereShader->use();

        vec3_t col3 = vec3(1.f);
        col3 /= real(lod.level + 1);
        auto col = glt::color(col3);

        glt::Uniforms us(*sphereShader);
        us.optional("mvpMatrix", gt.mvpMatrix());
        us.optional("mvMatrix", gt.mvMatrix());
        us.optional("normalMatrix", gt.normalMatrix());
        us.optional("ecLight", sphereUniforms.ecLightPos);
        us.optional("ecSpotDirection", sphereUniforms.ecSpotDir);
        us.optional("spotAngle", sphereUniforms.spotAngle);
        us.optional("color", col);
        us.optional("shininess", m.shininess);
        us.optional("gammaCorrection", indirect_rendering ? 1.f : GAMMA);

#ifdef GLDEBUG
        sphereShader->validate();
#endif
        sphereBatches[lod.level].draw();
    }
}

void
Renderer::renderBox(const glt::AABB &box)
{
    game.render_box(box);
}

void
Game::render_box(const glt::AABB &box)
{
    glt::GeometryTransform &gt = engine->renderManager().geometryTransform();

    GL_CALL(glCullFace, GL_FRONT);

    glt::SavePoint sp(gt.save());

    point3_t center = box.center();
    vec3_t dim = 0.5f * box.dimensions();

    //    sys::io::stderr() << "render box: " << center << "dim: " << dim <<
    //    sys::io::endl;

    gt.translate(center);
    gt.scale(dim);

    auto wallShader = engine->shaderManager().program("wall");
    ASSERT(wallShader);

    wallShader->use();

    glt::Uniforms us(*wallShader);
    us.optional("mvpMatrix", gt.mvpMatrix());
    us.optional("mvMatrix", gt.mvMatrix());
    us.optional("normalMatrix", gt.normalMatrix());
    us.optional("ecLight", wallUniforms.ecLightPos);
    us.optional("ecSpotDirection", wallUniforms.ecSpotDir);
    us.optional("spotAngle", wallUniforms.spotAngle);
    us.optional("gammaCorrection", indirect_rendering ? 1.f : GAMMA);

#ifdef GLDEBUG
    wallShader->validate(true);
#endif

    wallBatch.draw();

    GL_CALL(glCullFace, GL_BACK);
}

void
Renderer::renderConnection(const point3_t &a, const point3_t &b)
{
    game.render_con(a, b);
}

void
Game::render_con(const point3_t &a, const point3_t &b)
{

    Sphere s{};
    s.center = (a + b) * 0.5f;
    s.r = 0.1f;
    SphereModel m;
    m.color = glt::color(0x00, 0xFF, 0x00);
    render_sphere(s, m);

    GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, GL_LINE);
    GL_CALL(glLineWidth, 10.f);

    glt::GeometryTransform &gt = engine->renderManager().geometryTransform();

    glt::SavePoint sp(gt.save());

    gt.translate(a);
    mat3_t trans = mat3(b - a, vec3(0.f, 1.f, 0.f), vec3(0.f, 0.f, 1.f));
    gt.concat(trans);

    auto identityShader = engine->shaderManager().program("identity");

    identityShader->use();
    glt::Uniforms us(*identityShader);
    us.optional("mvpMatrix", gt.mvpMatrix());
    us.optional("color", CONNECTION_COLOR);
    us.optional("gammaCorrection", indirect_rendering ? 1.f : GAMMA);

    // sys::io::stderr() << "con: a = " << va << ", b = " << vb << sys::io::endl
    //           << "  a' = " << gt.transformPoint(vec3(0.f))
    //           << ", b' = " << gt.transformPoint(vec3(1.f, 0.f, 0.f))
    //           << sys::io::endl;

    lineBatch.draw();

    GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, GL_FILL);
}

void
Game::render_hud()
{
    /*
    const glt::FrameStatistics& fs = engine->renderManager().frameStatistics();

    sf::Color c(0, 255, 255, 200);

    uint32 height = 0;
    sf::Text txtFps = sf::Text(to_string(fs.avg_fps));
    txtFps.SetCharacterSize(12);
    txtFps.SetPosition(2, 0);
    txtFps.SetColor(sf::Color(255, 255, 0, 180));

    height += uint32(txtFps.GetGlobalBounds().Height) + 2;
    sf::Text txtSpeed("Sphere speed: " + to_string(sphere_speed) + " m/s");
    txtSpeed.SetCharacterSize(12);
    txtSpeed.SetColor(c);
    txtSpeed.SetPosition(2, height);

    height += uint32(txtSpeed.GetGlobalBounds().Height) + 2;
    sf::Text txtMass("Sphere mass: " + to_string(sphere_proto.m) + " kg");
    txtMass.SetCharacterSize(12);
    txtMass.SetColor(c);
    txtMass.SetPosition(2, height);

    height += uint32(txtMass.GetGlobalBounds().Height) + 2;
    sf::Text txtRad("Sphere radius: " + to_string(sphere_proto.r) + " m");
    txtRad.SetCharacterSize(12);
    txtRad.SetColor(c);
    txtRad.SetPosition(2, height);

    height += uint32(txtRad.GetGlobalBounds().Height) + 2;
    sf::Text txtAnimSpeed("Animation Speed: " + to_string(game_speed));
    txtAnimSpeed.SetCharacterSize(12);
    txtAnimSpeed.SetColor(c);
    txtAnimSpeed.SetPosition(2, height);

    height += uint32(txtAnimSpeed.GetGlobalBounds().Height) + 2;
    sf::Text txtInter(std::string("Interpolation: ") + (use_interpolation ? "on"
    : "off")); txtInter.SetCharacterSize(12); txtInter.SetColor(c);
    txtInter.SetPosition(2, height);

    height += uint32(txtInter.GetGlobalBounds().Height) + 2;
    sf::Text txtNumBalls(std::string("NO Spheres: ") +
    to_string(world.numSpheres())); txtNumBalls.SetCharacterSize(12);
    txtNumBalls.SetColor(c);
    txtNumBalls.SetPosition(2, height);

    height += uint32(txtNumBalls.GetGlobalBounds().Height) + 2;
    std::string render_stat = "FPS(Rendering): " + to_string(fs.rt_avg) + " " +
        to_string(fs.rt_current) + " " + to_string(fs.rt_min) + " " +
    to_string(fs.rt_max); sf::Text txtRenderTime(render_stat);
    txtRenderTime.SetCharacterSize(12);
    txtRenderTime.SetColor(c);
    txtRenderTime.SetPosition(2, height);

    // engine->window().window().SaveGLStates(); // FIXME

    engine->window().window().Draw(txtFps);
    engine->window().window().Draw(txtSpeed);
    engine->window().window().Draw(txtMass);
    engine->window().window().Draw(txtRad);
    engine->window().window().Draw(txtAnimSpeed);
    engine->window().window().Draw(txtInter);
    engine->window().window().Draw(txtNumBalls);
    engine->window().window().Draw(txtRenderTime);

    // engine->window().window().RestoreGLStates(); // FIXME
    */
}

void
Game::link(ge::Engine &e)
{
    e.events().animate.reg(*this, &Game::animate);
    e.events().render.reg(*this, &Game::renderScene);
    e.window().events().windowResized.reg(*this, &Game::windowResized);

    ge::CommandProcessor &proc = e.commandProcessor();

    proc.define(ge::makeCommand(
      "toggleUseInterpolation",
      "",
      [this](const ge::Event<ge::CommandEvent> & /*unused*/
      ) {
          use_interpolation = !use_interpolation;
          sys::io::stderr()
            << "use interpolation: " << (use_interpolation ? "yes" : "no")
            << sys::io::endl;
      }));

    proc.define(ge::makeCommand(
      "incWorldSolveIterations",
      "",
      [this](const ge::Event<ge::CommandEvent> & /*unused*/,
             ArrayView<const ge::CommandArg> args) {
          if (args[0].integer < 0 &&
              world.solve_iterations < size_t(-args[0].integer))
              world.solve_iterations = 0;
          else
              world.solve_iterations += int32_t(args[0].integer);
          sys::io::stderr()
            << "number of contact-solver iterations: " << world.solve_iterations
            << sys::io::endl;
      }));

    proc.define(ge::makeCommand(
      "toggleRenderByDistance",
      "",
      [this](const ge::Event<ge::CommandEvent> & /*unused*/) {
          world.render_by_distance = !world.render_by_distance;
          sys::io::stderr()
            << "render by distance: "
            << (world.render_by_distance ? "yes" : "no") << sys::io::endl;
      }));

    proc.define(ge::makeCommand(
      "toggleRenderSpheresInstanced",
      "",
      [this](const ge::Event<ge::CommandEvent> & /*unused*/) {
          render_spheres_instanced = !render_spheres_instanced;
          sys::io::stderr()
            << "instanced rendering: "
            << (render_spheres_instanced ? "yes" : "no") << sys::io::endl;
      }));

    proc.define(ge::makeCommand(
      "toggleIndirectRendering",
      "",
      [this](const ge::Event<ge::CommandEvent> & /*unused*/) {
          updateIndirectRendering(!indirect_rendering);
          sys::io::stderr()
            << "indirect rendering: " << (indirect_rendering ? "yes" : "no")
            << sys::io::endl;
      }));

    proc.define(
      ge::makeCommand("spawnSphere",
                      "",
                      [this](const ge::Event<ge::CommandEvent> & /*unused*/) {
                          spawn_sphere();
                      }));

    proc.define(ge::makeCommand(
      "incSphereRadius",
      "",
      [this](const ge::Event<ge::CommandEvent> & /*unused*/, double rad) {
          sphere_proto.r =
            std::max(0.05_r, sphere_proto.r + static_cast<math::real>(rad));
          update_sphere_mass();
      }));

    proc.define(ge::makeCommand(
      "incSphereSpeed",
      "",
      [this](const ge::Event<ge::CommandEvent> & /*unused*/, double inc) {
          sphere_speed =
            std::max(0.25_r, sphere_speed + static_cast<math::real>(inc));
      }));

    proc.define(ge::makeCommand(
      "incGameSpeed",
      "",
      [this](const ge::Event<ge::CommandEvent> & /*unused*/, double inc) {
          game_speed = std::max(0_r, game_speed + static_cast<math::real>(inc));
      }));

    mouse_look.camera(&camera);
    e.enablePlugin(camera);
    e.enablePlugin(mouse_look);
    camera.events().moved.reg(*this, &Game::constrainCameraMovement);
}

int
main(int argc, char *argv[])
{
    ge::Engine engine;
    engine.setDevelDataDir(CMAKE_CURRENT_SOURCE_DIR);
    ge::EngineOptions opts;
    Game game;

    opts.parse(&argc, &argv);
    opts.inits.reg(ge::Init, game, &Game::init);
    int32_t ec = engine.run(opts);
    return ec;
}
