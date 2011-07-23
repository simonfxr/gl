#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <climits>
#include <cstring>

#include "defs.h"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/ivec3.hpp"
#include "math/mat4.hpp"
#include "math/mat3.hpp"
#include "math/math.hpp"

#include "glt/utils.hpp"
#include "glt/primitives.hpp"
#include "glt/Mesh.hpp"
#include "glt/CubeMesh.hpp"
#include "glt/TextureRenderTarget.hpp"

#include "ge/GameWindow.hpp"
#include "ge/Engine.hpp"
#include "ge/Camera.hpp"

#include "sim.hpp"


using namespace math;

static const float SPHERE_DENSITY = 999.f;

static const vec4_t SPHERE_COLOR = vec4(0.f, 0.f, 1.f, 1.f);

static const vec3_t LIGHT_POS = vec3(11.f, 18.f, 11.f);

static const float FPS_RENDER_CYCLE = 1.f;

static const float CAMERA_SPHERE_RAD = 1.f;

static const uint32 NUM_BLUR_TEXTURES = 1;

static const float BLUR_TEXTURES_UPDATE_CYCLE = 0.04f / NUM_BLUR_TEXTURES;

static const float GAMMA = 1.8f;

static const glt::color CONNECTION_COLOR(0x00, 0xFF, 0x00);

static const uint32 AA_SAMPLES = 4;

static const uint32 SPHERE_LOD_MAX = 6;

struct SphereLOD {
    uint32 level;
};

// dont change layout (directly mapped to texture)
struct SphereInstance {
    point3_t pos;
    float rad;
    vec3_t col_rgb;
    float shininess;
} ATTRS(ATTR_PACKED);

struct Vertex {
    point4_t position;
    vec3_t normal;
};

DEFINE_VERTEX_DESC(Vertex,
    VERTEX_ATTR(Vertex, position),
    VERTEX_ATTR(Vertex, normal)
);

namespace {

typedef glt::Mesh<Vertex> Mesh;
typedef glt::CubeMesh<Vertex> CubeMesh;

std::ostream& LOCAL operator << (std::ostream& out, const vec3_t& v) {
    return out << "(" << v.x << ";" << v.y << ";" << v.z << ")";
}

} // namespace anon

struct Game {

    ge::Camera camera;

    CubeMesh wallBatch;
    Mesh sphereBatches[SPHERE_LOD_MAX];
    Mesh lineBatch;
    CubeMesh rectBatch;

    World world;
    float sphere_speed;
    Sphere sphere_proto;
    Renderer renderer;
    
    struct {
        vec3_t ecLightPos;
        vec3_t ecSpotDir;
        float spotAngle;
    } wallUniforms;
    
    struct {
        vec3_t ecLightPos;
        vec3_t ecSpotDir;
        float spotAngle;
    } sphereUniforms;

    glt::TextureRenderTarget *textureRenderTarget;
    ge::Engine *engine;

    float game_speed;

    bool use_interpolation;
    bool render_spheres_instanced;
    bool indirect_rendering;

    std::vector<SphereInstance> sphere_instances[SPHERE_LOD_MAX];

    Game();

    void updateIndirectRendering(bool indirect);
    void resizeRenderTargets();

    void init(const ge::Event<ge::InitEvent>&);
    void link(ge::Engine& e);

    void animate(const ge::Event<ge::EngineEvent>&);
    void renderScene(const ge::Event<ge::RenderEvent>&);
    void renderWorld(float dt);
    
    void windowResized(const ge::Event<ge::WindowResized>&);
    void handleInternalEvents();
    void spawn_sphere();

    SphereLOD calc_sphere_lod(const Sphere& s);
    void render_sphere(const Sphere& s, const SphereModel& m);
    void end_render_spheres();
    void render_box(const glt::AABB& box);
    void render_con(const point3_t& a, const point3_t& b);
    void render_hud();

    void update_sphere_mass();

    // commands
    void cmdToggleUseInterpolation(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&);
    void cmdIncWorldSolveIterations(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&);
    void cmdToggleRenderByDistance(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&);
    void cmdToggleRenderSpheresInstanced(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&);
    void cmdToggleIndirectRendering(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&);
    void cmdSpawnSphere(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&);
    void cmdIncSphereRadius(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&);
    void cmdIncSphereSpeed(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&);
    void cmdIncGameSpeed(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&);
};

Game::Game() :
    renderer(*this)
{}

void Game::init(const ge::Event<ge::InitEvent>& ev) {
    ge::Engine& e = ev.info.engine;
    engine = &e;

    link(e);
    
    GLuint vao;
    GL_CHECK(glGenVertexArrays(1, &vao));
    GL_CHECK(glBindVertexArray(vao));
    
    textureRenderTarget = 0;
    indirect_rendering = false;
    updateIndirectRendering(indirect_rendering);

    render_spheres_instanced = true;

    {
        Vertex v;
        v.normal = vec3(0.f, 0.f, 1.f);
        v.position = vec4(-1.f, -1.f, 0.f, 1.f); rectBatch.add(v);
        v.position = vec4( 1.f, -1.f, 0.f, 1.f); rectBatch.add(v);
        v.position = vec4( 1.f,  1.f, 0.f, 1.f); rectBatch.add(v);
        v.position = vec4(-1.f,  1.f, 0.f, 1.f); rectBatch.add(v);

        rectBatch.send();
    }

    e.gameLoop().ticksPerSecond(100);
    e.gameLoop().sync(true);
    
    use_interpolation = true;
    e.window().grabMouse(true);
    e.window().showMouseCursor(false);

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

    struct {
        int32 a, b;
    } sphere_params[SPHERE_LOD_MAX] = {
        { 12, 6 }, { 16, 8 },
        { 20, 10 }, { 24, 12 },
        { 26, 13 }, { 36, 18 }
    };

    for (uint32 i = 0; i < SPHERE_LOD_MAX; ++i) {
        glt::primitives::sphere(sphereBatches[i], 1.f, sphere_params[i].a, sphere_params[i].b);
        sphereBatches[i].send();
    }

    camera.frame.origin = vec3(-19.f, 10.f, +19.f);
    camera.frame.lookingAt(vec3(0.f, 10.f, 0.f));

    update_sphere_mass();

    Vertex vert;
    vert.normal = vec3(1.f, 0.f, 0.f);
    vert.position = vec4(vec3(0.f), 1.f);
    lineBatch.addVertex(vert);
    vert.position = vec4(1.f, 0.f, 0.f, 1.f);
    lineBatch.addVertex(vert);
    lineBatch.send();
    
    ev.info.success = true;
}

void Game::animate(const ge::Event<ge::EngineEvent>& ev) {
    float dt = game_speed * ev.info.engine.gameLoop().frameDuration();
    world.simulate(dt);
}

template <typename T>
static std::string to_string(T x) {
    std::stringstream out;
    out << x;
    return out.str();
}

std::ostream& operator <<(std::ostream& out, const vec4_t& a) {
    return out << "(" << a[0] << ", " << a[1] << ", " << a[2] << ", " << a[3] << ")";
}

void Game::windowResized(const ge::Event<ge::WindowResized>& ev) {
    uint32 width = ev.info.width;
    uint32 height = ev.info.height;

    // ge::Engine& e = ev.info.engine;

    // ASSERT(e.renderManager().renderTarget().viewport() == glt::Viewport());
    // ASSERT(e.renderManager().renderTarget().width() == width);
    // ASSERT(e.renderManager().renderTarget().height() == height);
    
    std::cerr << "new window dimensions: " << width << "x" << height << std::endl;

    resizeRenderTargets();
}

void Game::update_sphere_mass() {
    float V = 4.f / 3.f * math::PI * math::cubed(sphere_proto.r);
    sphere_proto.m = V * SPHERE_DENSITY;
}

static float rand1() {
    return rand() * (1.f / RAND_MAX);
}

static glt::color randomColor() {
    return glt::color(byte(rand1() * 255), byte(rand1() * 255), byte(rand1() * 255));
}

void Game::spawn_sphere() {
    vec3_t direction = camera.frame.localZ();
    sphere_proto.center = camera.frame.origin + direction * (sphere_proto.r + 1.1f);
    sphere_proto.v = direction * sphere_speed;

    SphereModel model;
    model.color = randomColor();
    model.shininess = 10.f + rand1() * 60;

    world.spawnSphere(sphere_proto, model);
}

void Game::renderScene(const ge::Event<ge::RenderEvent>& ev) {
    float interpolation = ev.info.interpolation;
    ge::Engine& e = ev.info.engine;

    engine = &e;
    
    glt::RenderManager& renderManager = e.renderManager();
    
    if (!use_interpolation)
        interpolation = 0.f;

    renderManager.activeRenderTarget()->clear(glt::RT_DEPTH_BUFFER);
    GL_CHECK(glEnable(GL_DEPTH_TEST));

    e.window().window().SetActive();

    float dt = interpolation * game_speed * e.gameLoop().frameDuration();

    renderWorld(dt);

    if (indirect_rendering) {
        GL_CHECK(glDisable(GL_DEPTH_TEST));
        
        renderManager.setActiveRenderTarget(&e.window().renderTarget());
        Ref<glt::ShaderProgram> postprocShader = e.shaderManager().program("postproc");
        ASSERT(postprocShader);
        postprocShader->use();
        
        textureRenderTarget->textureHandle().bind(0);
        glt::Uniforms(*postprocShader)
            .optional("gammaCorrection", GAMMA)
            .mandatory("textures", textureRenderTarget->textureHandle(), 0);
        
        rectBatch.drawArrays();        
    }

    render_hud();
}

void Game::renderWorld(float dt) {
    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
    const vec3_t eyeLightPos = gt.transformPoint(LIGHT_POS);
    const vec3_t spotDirection = normalize(vec3(0.f) - LIGHT_POS);
    const vec3_t ecSpotDirection = gt.transformVector(spotDirection);
        
    wallUniforms.ecLightPos = eyeLightPos;
    wallUniforms.ecSpotDir = ecSpotDirection;
    wallUniforms.spotAngle = 0.91;

    sphereUniforms.ecLightPos = eyeLightPos;
    sphereUniforms.ecSpotDir = ecSpotDirection;
    sphereUniforms.spotAngle = 0.91;
        
    world.render(renderer, dt);
}

const glt::ViewFrustum& Renderer::frustum() {
    return game.engine->renderManager().viewFrustum();
}

const glt::Frame& Renderer::camera() {
    return game.camera.frame;
}


void Game::updateIndirectRendering(bool indirect) {
    glt::RenderTarget *rt;

    if (indirect) {

        if (textureRenderTarget == 0) {
            uint32 w = engine->window().window().GetWidth();
            uint32 h = engine->window().window().GetHeight();
            textureRenderTarget = new glt::TextureRenderTarget(w, h, glt::RT_COLOR_BUFFER | glt::RT_DEPTH_BUFFER);
        }
        
        rt = textureRenderTarget;
    } else {
        rt = &engine->window().renderTarget();
    }

    indirect_rendering = indirect;
    
    engine->renderManager().setActiveRenderTarget(0);
    resizeRenderTargets();
    engine->renderManager().setDefaultRenderTarget(rt);
}

void Game::resizeRenderTargets() {
    uint32 width = engine->window().window().GetWidth();
    uint32 height = engine->window().window().GetHeight();
    
    uint32 stride = 1;
    while (stride * stride < AA_SAMPLES)
        ++stride;

    if (indirect_rendering)
        textureRenderTarget->resize(width * stride, height * stride);
}

void Renderer::renderSphere(const Sphere& s, const SphereModel& m) {
    game.render_sphere(s, m);
}

void Renderer::endRenderSpheres() {
    game.end_render_spheres();
}

void Game::end_render_spheres() {
    if (render_spheres_instanced) {

        for (uint32 lod = 0; lod < SPHERE_LOD_MAX; ++lod) {
            uint32 num = sphere_instances[lod].size();

            if (num == 0)
                continue;

            glt::TextureHandle sphereMap(glt::Texture1D);
            sphereMap.bind();

            GL_CHECK(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT));
            GL_CHECK(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GL_CHECK(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            GL_CHECK(glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, num * 2, 0, GL_RGBA, GL_FLOAT, &sphere_instances[lod][0]));

            sphere_instances[lod].clear();

            Ref<glt::ShaderProgram> sphereInstancedShader = engine->shaderManager().program("sphereInstanced");

            ASSERT(sphereInstancedShader);

            sphereInstancedShader->use();

            sphereMap.bind(0);

            glt::GeometryTransform& gt = engine->renderManager().geometryTransform();

            glt::Uniforms(*sphereInstancedShader)
                .optional("normalMatrix", gt.normalMatrix())
                .optional("vMatrix", gt.mvMatrix())
                .optional("pMatrix", gt.projectionMatrix())
                .optional("ecLight", sphereUniforms.ecLightPos)
                .optional("gammaCorrection", indirect_rendering ? 1.f : GAMMA)
                .mandatory("instanceData", sphereMap, 0);

            sphereBatches[lod].drawArraysInstanced(num);

            sphereMap.free();
        }
    }
}

SphereLOD Game::calc_sphere_lod(const Sphere& s) {
    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
    vec3_t ecCoord = gt.transformPoint(s.center);

    // near upper right corner of frustm (view coord)
    vec4_t nur_corner = gt.inverseProjectionMatrix() * vec4(-1.f, -1.f, -1.f, 1.f);
    float x_max = abs(nur_corner.x);
    float y_max = abs(nur_corner.y);
    float z_min = abs(nur_corner.z);
    
    float size = min(x_max, y_max);
        
    // calculate lod, use projected radius on screen
    float r_proj = s.r / ecCoord.z * z_min;
    float screen_rad = r_proj / size;

    screen_rad *= 2;

    SphereLOD lod;
    lod.level = uint32(screen_rad * SPHERE_LOD_MAX);
    if (lod.level >= SPHERE_LOD_MAX)
        lod.level = SPHERE_LOD_MAX - 1;
    return lod;
}

void Game::render_sphere(const Sphere& s, const SphereModel& m) {

    SphereLOD lod = calc_sphere_lod(s);

    if (render_spheres_instanced) {

        SphereInstance inst;
        inst.pos = s.center;
        inst.rad = s.r;
        inst.col_rgb = vec3(m.color.vec4());
        inst.shininess = m.shininess;
        
        sphere_instances[lod.level].push_back(inst);
        
    } else {
        glt::GeometryTransform& gt = engine->renderManager().geometryTransform();
        const vec3_t& pos = s.center;
        glt::SavePoint sp(gt.save());
        gt.translate(vec3(pos.x, pos.y, pos.z));
        gt.scale(vec3(s.r));

        Ref<glt::ShaderProgram> sphereShader = engine->shaderManager().program("sphere");
        ASSERT(sphereShader);

        sphereShader->use();

        vec3_t col3 = vec3(1.f);
        col3 /= lod.level + 1;
        glt::color col = glt::color(col3);
        
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

void Renderer::renderBox(const glt::AABB& box) {
    game.render_box(box);
}

void Game::render_box(const glt::AABB& box) {
    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();

    GL_CHECK(glCullFace(GL_FRONT));

    glt::SavePoint sp(gt.save());

    point3_t center = box.center();
    vec3_t diff = 0.5f * box.dimensions();

    gt.translate(vec3(center.x, center.y, center.z));
    gt.scale(vec3(diff.x, diff.y, diff.z));

    Ref<glt::ShaderProgram> wallShader = engine->shaderManager().program("wall");
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

    GL_CHECK(glCullFace(GL_BACK));    
}

void Renderer::renderConnection(const point3_t& a, const point3_t& b) {
    game.render_con(a, b);
}

void Game::render_con(const point3_t& a, const point3_t& b) {

    Sphere s;
    s.center = (a + b) * 0.5f;
    s.r = 0.1f;
    SphereModel m;
    m.color = glt::color(0x00, 0xFF, 0x00);
    render_sphere(s, m);

    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    GL_CHECK(glLineWidth(10.f));

    glt::GeometryTransform& gt = engine->renderManager().geometryTransform();

    glt::SavePoint sp(gt.save());

    gt.translate(a);
    mat3_t trans = mat3(b - a,
                        vec3(0.f, 1.f, 0.f),
                        vec3(0.f, 0.f, 1.f));
    gt.concat(trans);

    Ref<glt::ShaderProgram> identityShader = engine->shaderManager().program("identity");

    identityShader->use();
    glt::Uniforms us(*identityShader);
    us.optional("mvpMatrix", gt.mvpMatrix());
    us.optional("color", CONNECTION_COLOR);
    us.optional("gammaCorrection", indirect_rendering ? 1.f : GAMMA);

    // std::cerr << "con: a = " << va << ", b = " << vb << std::endl
    //           << "  a' = " << gt.transformPoint(vec3(0.f))
    //           << ", b' = " << gt.transformPoint(vec3(1.f, 0.f, 0.f))
    //           << std::endl;

    lineBatch.draw();

    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
}

void Game::render_hud() {
    const glt::FrameStatistics& fs = engine->renderManager().frameStatistics();
    
    sf::Color c(0, 255, 255, 200);

    uint32 height = 0;
    sf::Text txtFps = sf::Text(to_string(fs.avg_fps));
    txtFps.SetCharacterSize(12);
    txtFps.SetPosition(2, 0);
    txtFps.SetColor(sf::Color(255, 255, 0, 180));

    height += txtFps.GetRect().Height + 2;
    sf::Text txtSpeed("Sphere speed: " + to_string(sphere_speed) + " m/s");
    txtSpeed.SetCharacterSize(12);
    txtSpeed.SetColor(c);
    txtSpeed.SetPosition(2, height);

    height += txtSpeed.GetRect().Height + 2;    
    sf::Text txtMass("Sphere mass: " + to_string(sphere_proto.m) + " kg");
    txtMass.SetCharacterSize(12);
    txtMass.SetColor(c);
    txtMass.SetPosition(2, height);

    height += txtMass.GetRect().Height + 2;
    sf::Text txtRad("Sphere radius: " + to_string(sphere_proto.r) + " m");
    txtRad.SetCharacterSize(12);
    txtRad.SetColor(c);
    txtRad.SetPosition(2, height);

    height += txtRad.GetRect().Height + 2;
    sf::Text txtAnimSpeed("Animation Speed: " + to_string(game_speed));
    txtAnimSpeed.SetCharacterSize(12);
    txtAnimSpeed.SetColor(c);
    txtAnimSpeed.SetPosition(2, height);

    height += txtAnimSpeed.GetRect().Height + 2;
    sf::Text txtInter(std::string("Interpolation: ") + (use_interpolation ? "on" : "off"));
    txtInter.SetCharacterSize(12);
    txtInter.SetColor(c);
    txtInter.SetPosition(2, height);

    height += txtInter.GetRect().Height + 2;
    sf::Text txtNumBalls(std::string("NO Spheres: ") + to_string(world.numSpheres()));
    txtNumBalls.SetCharacterSize(12);
    txtNumBalls.SetColor(c);
    txtNumBalls.SetPosition(2, height);

    height += txtNumBalls.GetRect().Height + 2;
    std::string render_stat = "FPS(Rendering): " + to_string(fs.rt_avg) + " " +
        to_string(fs.rt_current) + " " + to_string(fs.rt_min) + " " + to_string(fs.rt_max);
    sf::Text txtRenderTime(render_stat);
    txtRenderTime.SetCharacterSize(12);
    txtRenderTime.SetColor(c);
    txtRenderTime.SetPosition(2, height);

    engine->window().window().SaveGLStates();
    
    engine->window().window().Draw(txtFps);
    engine->window().window().Draw(txtSpeed);
    engine->window().window().Draw(txtMass);
    engine->window().window().Draw(txtRad);
    engine->window().window().Draw(txtAnimSpeed);
    engine->window().window().Draw(txtInter);
    engine->window().window().Draw(txtNumBalls);
    engine->window().window().Draw(txtRenderTime);
    
    engine->window().window().RestoreGLStates();
}

void Game::link(ge::Engine& e) {
    e.events().animate.reg(ge::makeEventHandler(this, &Game::animate));
    e.events().render.reg(ge::makeEventHandler(this, &Game::renderScene));
    e.window().events().windowResized.reg(ge::makeEventHandler(this, &Game::windowResized));

    ge::CommandProcessor& proc = e.commandProcessor();
    
    proc.define(ge::makeCommand(this, &Game::cmdToggleUseInterpolation, ge::NULL_PARAMS,
                                "toggleUseInterpolation"));

    proc.define(ge::makeCommand(this, &Game::cmdIncWorldSolveIterations, PARAM_ARRAY(ge::IntegerParam),
                                "incWorldSolveIterations"));

    proc.define(ge::makeCommand(this, &Game::cmdToggleRenderByDistance, ge::NULL_PARAMS,
                                "toggleRenderByDistance"));
    
    proc.define(ge::makeCommand(this, &Game::cmdToggleRenderSpheresInstanced, ge::NULL_PARAMS,
                                "toggleRenderSpheresInstanced"));

    proc.define(ge::makeCommand(this, &Game::cmdToggleIndirectRendering, ge::NULL_PARAMS,
                                "toggleIndirectRendering"));
    
    proc.define(ge::makeCommand(this, &Game::cmdSpawnSphere, ge::NULL_PARAMS,
                                "spawnSphere"));

    proc.define(ge::makeCommand(this, &Game::cmdIncSphereRadius, PARAM_ARRAY(ge::NumberParam),
                                "incSphereRadius"));
    
    proc.define(ge::makeCommand(this, &Game::cmdIncSphereSpeed, PARAM_ARRAY(ge::NumberParam),
                                "incSphereSpeed"));

    proc.define(ge::makeCommand(this, &Game::cmdIncGameSpeed, PARAM_ARRAY(ge::NumberParam),
                                "incGameSpeed"));

    camera.registerWith(e);
    camera.registerCommands(e.commandProcessor());
}

void Game::cmdToggleUseInterpolation(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&) {
    use_interpolation = !use_interpolation;
    std::cerr << "use interpolation: " << (use_interpolation ? "yes" : "no") << std::endl;
}

void Game::cmdIncWorldSolveIterations(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>& args) {
    if (args[0].integer < 0 && world.solve_iterations < - args[0].integer)
        world.solve_iterations = 0;
    else
        world.solve_iterations += args[0].integer;
    std::cerr << "number of contact-solver iterations: " << world.solve_iterations << std::endl;
}

void Game::cmdToggleRenderByDistance(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&) {
    world.render_by_distance = !world.render_by_distance;
    std::cerr << "render by distance: " << (world.render_by_distance ? "yes" : "no") << std::endl;
}

void Game::cmdToggleRenderSpheresInstanced(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&) {
    render_spheres_instanced = !render_spheres_instanced;
    std::cerr << "instanced rendering: " << (render_spheres_instanced ? "yes" : "no") << std::endl;
}

void Game::cmdToggleIndirectRendering(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&) {
    updateIndirectRendering(!indirect_rendering);
    std::cerr << "indirect rendering: " << (indirect_rendering ? "yes" : "no") << std::endl;
}

void Game::cmdSpawnSphere(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>&) {
    spawn_sphere();
}

void Game::cmdIncSphereRadius(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>& args) {
    sphere_proto.r = std::max(0.05f, sphere_proto.r + (float) args[0].number);
}

void Game::cmdIncSphereSpeed(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>& args) {
    sphere_speed = std::max(0.25f, sphere_speed + (float) args[0].number);
}

void Game::cmdIncGameSpeed(const ge::Event<ge::CommandEvent>&, const Array<ge::CommandArg>& args) {
    game_speed = std::max(0.f, game_speed + (float) args[0].number);
}

int main(int argc, char *argv[]) {
    ge::Engine engine;
    ge::EngineOptions opts;
    Game game;
    
    opts.parse(&argc, &argv);
    opts.inits.reg(ge::Init, ge::makeEventHandler(&game, &Game::init));
    return engine.run(opts);    
}
