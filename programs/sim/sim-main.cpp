#include "sim.hpp"

#include "ge/Anim.hpp"
#include "ge/Command.hpp"
#include "ge/GameWindow.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/Mesh.hpp"
#include "glt/TextureRenderTarget.hpp"
#include "glt/primitives.hpp"
#include "glt/utils.hpp"
#include "math/fvec.hpp"
#include "math/ivec.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/real.hpp"

#include <bl/rand.hpp>
#include <bl/rand/uniform.hpp>
#include <bl/string.hpp>

#include <algorithm>
#include <bl/vec.hpp>

using namespace math;
using namespace bl::literals;

inline constexpr real SPHERE_DENSITY = 999_r;

// inline constexpr vec4 SPHERE_COLOR = vec4(0.f, 0.f, 1.f, 1.f);

inline constexpr vec3 LIGHT_POS = vec3(11.f, 18.f, 11.f);

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
struct SphereInstance
{
    vec3 pos;
    float rad;
    vec3 col_rgb;
    float shininess;
};
#elif defined(SPHERE_INSTANCED_ARRAY)
struct SphereInstance
{
    mat4_t mvMatrix;
    vec4 colorShininess;
};
#else
#    error "no SPHERE_INSTANCED_* specified"
#endif

DEF_GL_MAPPED_TYPE(Vertex, (vec4, position), (vec3, normal))

namespace {

using Mesh = glt::Mesh<Vertex>;
using CubeMesh = glt::CubeMesh<Vertex>;

} // namespace

struct Game final : ge::Anim3D<Game>
{

    CubeMesh wallBatch;
    Mesh sphereBatches[SPHERE_LOD_MAX];
    Mesh lineBatch;
    CubeMesh rectBatch;

    World world;
    float sphere_speed{};
    Sphere sphere_proto{};
    Renderer renderer;

    bl::rand::gen rng;

    struct
    {
        vec3 ecLightPos;
        vec3 ecSpotDir;
        float spotAngle;
    } wallUniforms{};

    struct
    {
        vec3 ecLightPos;
        vec3 ecSpotDir;
        float spotAngle;
    } sphereUniforms{};

    std::shared_ptr<glt::TextureRenderTarget> textureRenderTarget{};

    float game_speed{};

    bool use_interpolation{};
    bool render_spheres_instanced{};
    bool indirect_rendering{};

    bl::vec<SphereInstance> sphere_instances[SPHERE_LOD_MAX];

    Game();

    void updateIndirectRendering(bool indirect);
    void resizeRenderTargets();

    void init(const ge::Event<ge::InitEvent> & /*ev*/);
    void initCommands();

    void constrainCameraMovement(const ge::Event<ge::CameraMoved> & /*ev*/);
    void animate(const ge::Event<ge::AnimationEvent> & /*ev*/) override;
    void render(const ge::Event<ge::RenderEvent> & /*ev*/) override;
    void renderWorld(float dt);

    void windowResized(const ge::Event<ge::WindowResized> & /*ev*/);
    void handleInternalEvents();
    void spawn_sphere();

    SphereLOD calc_sphere_lod(const Sphere &s);
    void render_sphere(const Sphere &s, const SphereModel &m);
    void end_render_spheres();
    void render_box(const glt::AABB &box);
    void render_con(const point3 &a, const point3 &b);
    void render_hud();

    void update_sphere_mass();
};

Game::Game() : renderer(*this) {}

void
Game::init(const ge::Event<ge::InitEvent> &ev)
{
    initCommands();

    engine->window().events().windowResized.reg(*this, &Game::windowResized);
    get<ge::Camera>().events().moved.reg(*this, &Game::constrainCameraMovement);

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

    engine->gameLoop().ticks(200);
    engine->gameLoop().syncDraw(false);

    use_interpolation = true;

    if (!world.init())
        return;

    world.render_by_distance = true;

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

    get<ge::Camera>().frame().origin = vec3(0.f, 0.f, 0.f);

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
    if (!world.canMoveCamera(ev.info.camera.frame().origin,
                             ev.info.allowed_step))
        ev.info.allowed_step = vec3(0.f);
}

void
Game::animate(const ge::Event<ge::AnimationEvent> &ev)
{
    auto dt = game_speed * ev.info.engine.gameLoop().tickDuration();
    world.simulate(static_cast<real>(dt));
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

    sys::io::stderr() << "new window dimensions: "_k << width << "x"_k << height
                      << "\n"_k;

    resizeRenderTargets();
}

void
Game::update_sphere_mass()
{
    float V = 4.f / 3.f * math::PI * math::cubed(sphere_proto.r);
    sphere_proto.m = V * SPHERE_DENSITY;
}

static glt::color
randomColor(bl::rand::rng auto &&rng)
{
    return glt::color{ bl::rand::sample_uniform<vec3>(bl_fwd(rng)) };
}

void
Game::spawn_sphere()
{
    auto &camera = get<ge::Camera>();
    auto direction = -camera.frame().localZ();
    sphere_proto.center =
      camera.frame().origin + direction * (sphere_proto.r + 1.1f);
    sphere_proto.v = direction * sphere_speed;

    SphereModel model{};
    model.color = randomColor(rng);
    model.shininess = 10.f + bl::rand::sample_uniform<real>(rng) * 60;

    world.spawnSphere(sphere_proto, model);
}

void
Game::render(const ge::Event<ge::RenderEvent> &ev)
{
    float interpolation = ev.info.interpolation;
    ge::Engine &e = ev.info.engine;

    glt::RenderManager &renderManager = e.renderManager();

    if (!use_interpolation)
        interpolation = 0.f;

    renderManager.activeRenderTarget()->clear(glt::RT_DEPTH_BUFFER);
    GL_CALL(glEnable, GL_DEPTH_TEST);

    //    e.window().window().setActive();

    auto dt = static_cast<real>(interpolation * game_speed *
                                e.gameLoop().tickDuration());

    renderWorld(dt);

    if (indirect_rendering) {
        GL_CALL(glDisable, GL_DEPTH_TEST);

        renderManager.setActiveRenderTarget(e.window().renderTarget());
        auto postprocShader = e.shaderManager().program("postproc"_k);
        ASSERT(postprocShader);
        postprocShader->use();

        textureRenderTarget->sampler().bind(0);
        glt::Uniforms(*postprocShader)
          .optional("gammaCorrection"_k, GAMMA)
          .mandatory("textures"_k,
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
    const vec3 eyeLightPos = gt.transformPoint(LIGHT_POS);
    const vec3 spotDirection = normalize(vec3(0.f) - LIGHT_POS);
    const vec3 ecSpotDirection = gt.transformVector(spotDirection);

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
    return game.get<ge::Camera>().frame();
}

void
Game::updateIndirectRendering(bool indirect)
{
    std::shared_ptr<glt::RenderTarget> rt;

    if (indirect) {

        if (textureRenderTarget == nullptr) {
            auto w = engine->window().windowWidth();
            auto h = engine->window().windowHeight();
            glt::TextureRenderTarget::Params ps;
            ps.buffers = glt::RT_COLOR_BUFFER | glt::RT_DEPTH_BUFFER;
            textureRenderTarget =
              std::make_shared<glt::TextureRenderTarget>(ps);
            textureRenderTarget->resize(w, h);
        }

        rt = textureRenderTarget;
    } else {
        rt = engine->window().renderTarget();
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
                    sphere_instances[lod].data());

            sphere_instances[lod].clear();

            auto sphereInstancedShader =
              engine->shaderManager().program("sphereInstanced"_k);
            ASSERT(sphereInstancedShader);
            sphereInstancedShader->use();

            glt::GeometryTransform &gt =
              engine->renderManager().geometryTransform();
            glt::Uniforms(*sphereInstancedShader)
              .optional("normalMatrix"_k, gt.normalMatrix())
              .optional("vMatrix"_k, gt.mvMatrix())
              .optional("pMatrix"_k, gt.projectionMatrix())
              .optional("ecLight"_k, sphereUniforms.ecLightPos)
              .optional("gammaCorrection"_k, indirect_rendering ? 1.f : GAMMA)
              .mandatory("instanceData"_k, glt::Sampler(sphereMap, 0));

            sphereBatches[lod].drawInstanced(num);
            sphereMap.unbind(0);
            sphereMap.free();

#elif defined(SPHERE_INSTANCED_ARRAY)

            auto sphereInstanced2Shader =
              engine->shaderManager().program("sphereInstanced2"_k);
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
                           "mvMatrix"_k);
            ASSERT(attr_mvMatrix >= 0);
            GLint attr_colorShininess;
            GL_ASSIGN_CALL(attr_colorShininess,
                           glGetAttribLocation,
                           sphereInstanced2Shader->program(),
                           "colorShininess"_k);
            ASSERT(attr_colorShininess >= 0);

            for (uint32 col = 0; col < 4; ++col) {
                GL_CALL(
                  glVertexAttribPointer,
                  attr_mvMatrix + col,
                  4,
                  GL_FLOAT,
                  GL_FALSE,
                  sizeof(vec4),
                  GLT_FIELD_OFFSET(SphereInstance, mvMatrix.columns[col]));
                GL_CALL(glVertexAttribDivisor, attr_mvMatrix + col, 1);
                GL_CALL(glEnableVertexAttribArray, attr_mvMatrix + col);
            }

            GL_CALL(glVertexAttribPointer,
                    attr_colorShininess,
                    4,
                    GL_FLOAT,
                    GL_FALSE,
                    sizeof(SphereInstance),
                    GLT_FIELD_OFFST(SphereInstance, colorShininess));
            GL_CALL(glVertexAttribDivisor, attr_colorShininess, 1);
            GL_CALL(glEnableVertexAttribArray, attr_colorShininess);

            GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

            glt::GeometryTransform &gt =
              engine->renderManager().geometryTransform();
            glt::Uniforms(*sphereInstanced2Shader)
              .optional("pMatrix"_k, gt.projectionMatrix())
              .optional("ecLight"_k, sphereUniforms.ecLightPos)
              .optional("gammaCorrection"_k, indirect_rendering ? 1.f : GAMMA);

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
    vec3 ecCoord = gt.transformPoint(s.center);

    // near upper right corner of frustm (view coord)
    vec4 nur_corner = gt.inverseProjectionMatrix()(vec4(-1.f, -1.f, -1.f, 1.f));

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
    lod.level = static_cast<size_t>(screen_rad * SPHERE_LOD_MAX);
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
        ASSERT(lod.level < SPHERE_LOD_MAX);
        sphere_instances[lod.level].push_back(inst);

    } else {
        glt::GeometryTransform &gt =
          engine->renderManager().geometryTransform();
        const vec3 &pos = s.center;
        auto sp = gt.save();
        gt.translate(pos);
        gt.scale(vec3(s.r));

        auto sphereShader = engine->shaderManager().program("sphere"_k);
        ASSERT(sphereShader);

        sphereShader->use();

        vec3 col3 = vec3(1.f);
        col3 /= static_cast<real>(lod.level + 1);
        auto col = glt::color(col3);

        glt::Uniforms us(*sphereShader);
        us.optional("mvpMatrix"_k, gt.mvpMatrix());
        us.optional("mvMatrix"_k, gt.mvMatrix());
        us.optional("normalMatrix"_k, gt.normalMatrix());
        us.optional("ecLight"_k, sphereUniforms.ecLightPos);
        us.optional("ecSpotDirection"_k, sphereUniforms.ecSpotDir);
        us.optional("spotAngle"_k, sphereUniforms.spotAngle);
        us.optional("color"_k, col);
        us.optional("shininess"_k, m.shininess);
        us.optional("gammaCorrection"_k, indirect_rendering ? 1.f : GAMMA);

#if ENABLE_GLDEBUG_P
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

    auto sp = gt.save();

    point3 center = box.center();
    vec3 dim = 0.5f * box.dimensions();

    //    sys::io::stderr() << "render box: " << center << "dim: " << dim <<
    //    sys::io::endl;

    gt.translate(center);
    gt.scale(dim);

    auto wallShader = engine->shaderManager().program("wall"_k);
    ASSERT(wallShader);

    wallShader->use();

    glt::Uniforms us(*wallShader);
    us.optional("mvpMatrix"_k, gt.mvpMatrix());
    us.optional("mvMatrix"_k, gt.mvMatrix());
    us.optional("normalMatrix"_k, gt.normalMatrix());
    us.optional("ecLight"_k, wallUniforms.ecLightPos);
    us.optional("ecSpotDirection"_k, wallUniforms.ecSpotDir);
    us.optional("spotAngle"_k, wallUniforms.spotAngle);
    us.optional("gammaCorrection"_k, indirect_rendering ? 1.f : GAMMA);

#if ENABLE_GLDEBUG_P
    wallShader->validate(true);
#endif

    wallBatch.draw();

    GL_CALL(glCullFace, GL_BACK);
}

void
Renderer::renderConnection(const point3 &a, const point3 &b)
{
    game.render_con(a, b);
}

void
Game::render_con(const point3 &a, const point3 &b)
{

    Sphere s{};
    s.center = (a + b) * 0.5f;
    s.r = 0.1f;
    SphereModel m{};
    m.color = glt::color(0x00, 0xFF, 0x00);
    render_sphere(s, m);

    GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, GL_LINE);
    GL_CALL(glLineWidth, 10.f);

    glt::GeometryTransform &gt = engine->renderManager().geometryTransform();

    auto sp = gt.save();

    gt.translate(a);
    mat3_t trans = mat3(b - a, vec3(0.f, 1.f, 0.f), vec3(0.f, 0.f, 1.f));
    gt.concat(trans);

    auto identityShader = engine->shaderManager().program("identity"_k);

    identityShader->use();
    glt::Uniforms us(*identityShader);
    us.optional("mvpMatrix"_k, gt.mvpMatrix());
    us.optional("color"_k, CONNECTION_COLOR);
    us.optional("gammaCorrection"_k, indirect_rendering ? 1.f : GAMMA);

    // sys::io::stderr() << "con: a = " << va << "_k, b = " << vb << "\n"
    //           << "  a' = " << gt.transformPoint(vec3(0.f))
    //           << , b' = " << gt.transformPoint(vec3(1.f, 0.f, 0.f))
    //           << "\n";

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
    sf::Text txtInter(bl::string("Interpolation: ") + (use_interpolation ? "on"
    : "off")); txtInter.SetCharacterSize(12); txtInter.SetColor(c);
    txtInter.SetPosition(2, height);

    height += uint32(txtInter.GetGlobalBounds().Height) + 2;
    sf::Text txtNumBalls(bl::string("NO Spheres: ") +
    to_string(world.numSpheres())); txtNumBalls.SetCharacterSize(12);
    txtNumBalls.SetColor(c);
    txtNumBalls.SetPosition(2, height);

    height += uint32(txtNumBalls.GetGlobalBounds().Height) + 2;
    bl::string render_stat = "FPS(Rendering): " + to_string(fs.rt_avg) + " " +
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
Game::initCommands()
{
    auto &proc = engine->commandProcessor();

    proc.define(ge::makeCommand(
      "toggleUseInterpolation"_k,
      ""_k,
      [this](const ge::Event<ge::CommandEvent> & /*unused*/
      ) {
          use_interpolation = !use_interpolation;
          sys::io::stderr() << "use interpolation: "_k
                            << (use_interpolation ? "yes"_k : "no"_k) << "\n"_k;
      }));

    proc.define(ge::makeCommand(
      "incWorldSolveIterations"_k,
      ""_k,
      [this](const ge::Event<ge::CommandEvent> & /*unused*/,
             bl::span<const ge::CommandArg> args) {
          if (args[0].integer < 0 &&
              world.solve_iterations < static_cast<size_t>(-args[0].integer))
              world.solve_iterations = 0;
          else
              world.solve_iterations += static_cast<int32_t>(args[0].integer);
          sys::io::stderr() << "number of contact-solver iterations: "_k
                            << world.solve_iterations << "\n"_k;
      }));

    proc.define(ge::makeCommand(
      "toggleRenderByDistance"_k,
      ""_k,
      [this](const ge::Event<ge::CommandEvent> & /*unused*/) {
          world.render_by_distance = !world.render_by_distance;
          sys::io::stderr()
            << "render by distance: "_k
            << (world.render_by_distance ? "yes"_k : "no"_k) << "\n"_k;
      }));

    proc.define(ge::makeCommand(
      "toggleRenderSpheresInstanced"_k,
      ""_k,
      [this](const ge::Event<ge::CommandEvent> & /*unused*/) {
          render_spheres_instanced = !render_spheres_instanced;
          sys::io::stderr()
            << "instanced rendering: "_k
            << (render_spheres_instanced ? "yes"_k : "no"_k) << "\n"_k;
      }));

    proc.define(ge::makeCommand(
      "toggleIndirectRendering"_k,
      ""_k,
      [this](const ge::Event<ge::CommandEvent> & /*unused*/) {
          updateIndirectRendering(!indirect_rendering);
          sys::io::stderr()
            << "indirect rendering: "_k
            << (indirect_rendering ? "yes"_k : "no"_k) << "\n"_k;
      }));

    proc.define(
      ge::makeCommand("spawnSphere"_k,
                      ""_k,
                      [this](const ge::Event<ge::CommandEvent> & /*unused*/) {
                          spawn_sphere();
                      }));

    proc.define(ge::makeCommand(
      "incSphereRadius"_k,
      ""_k,
      [this](const ge::Event<ge::CommandEvent> & /*unused*/, double rad) {
          sphere_proto.r =
            std::max(0.05_r, sphere_proto.r + static_cast<math::real>(rad));
          update_sphere_mass();
      }));

    proc.define(ge::makeCommand(
      "incSphereSpeed"_k,
      ""_k,
      [this](const ge::Event<ge::CommandEvent> & /*unused*/, double inc) {
          sphere_speed =
            std::max(0.25_r, sphere_speed + static_cast<math::real>(inc));
      }));

    proc.define(ge::makeCommand(
      "incGameSpeed"_k,
      ""_k,
      [this](const ge::Event<ge::CommandEvent> & /*unused*/, double inc) {
          game_speed = std::max(0_r, game_speed + static_cast<math::real>(inc));
      }));
}

int
main(int argc, char *argv[])
{
    Game game;
    game.engine->setDevelDataDir(
      bl::string_view_literal(PP_TOSTR(CMAKE_CURRENT_SOURCE_DIR)));
    return game.main(argc, argv);
}
