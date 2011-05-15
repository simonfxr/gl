#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <climits>
#include <cstring>

#include <SFML/Graphics.hpp>

#include "defs.h"

#include "fs/fs.hpp"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/ivec3.hpp"
#include "math/mat4.hpp"
#include "math/mat3.hpp"
#include "math/math.hpp"

#include "glt/utils.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/Uniforms.hpp"
#include "glt/color.hpp"
#include "glt/Frame.hpp"
#include "glt/GeometryTransform.hpp"
#include "glt/Transformations.hpp"
#include "glt/RenderManager.hpp"
#include "glt/TextureRenderTarget.hpp"

#include "ge/GameWindow.hpp"

#include "sim.hpp"

#ifdef MESH_GENBATCH
#include "glt/GenBatch.hpp"
#else
#include "glt/Mesh.hpp"
#include "glt/CubeMesh.hpp"
#endif

using namespace math;

static const float SPHERE_DENSITY = 999.f;

static const vec4_t SPHERE_COLOR = vec4(0.f, 0.f, 1.f, 1.f);

static const vec3_t LIGHT_POS = vec3(11.f, 18.f, 11.f);

static const float FPS_RENDER_CYCLE = 1.f;

static const float CAMERA_STEP = 0.1f;

static const float CAMERA_SPHERE_RAD = 1.f;

static const uint32 NUM_BLUR_TEXTURES = 1;

static const float BLUR_TEXTURES_UPDATE_CYCLE = 0.04f / NUM_BLUR_TEXTURES;

static const float PROJ_Z_MIN = 1.f;
static const float PROJ_Z_MAX = 100.f;

static const float GAMMA = 1.8f;

static const glt::color CONNECTION_COLOR(0x00, 0xFF, 0x00);

static const uint32 AA_SAMPLES = 4;

static const uint32 SPHERE_LOD_MAX = 6;

struct SphereLOD {
    uint32 level;
};

namespace {

// dont change layout (directly mapped to texture)
struct SphereInstance {
    point3_t pos;
    float rad;
    vec3_t col_rgb;
    float shininess;
};

struct Vertex {
    point4_t position;
    vec3_t normal;
};

#ifdef MESH_GENBATCH

glt::Attr vertexAttrArray[] = {
    glt::attr::vec4(offsetof(Vertex, position)),
    glt::attr::vec3(offsetof(Vertex, normal))
};

glt::Attrs<Vertex> vertexAttrs(ARRAY_LENGTH(vertexAttrArray), vertexAttrArray);

typedef glt::GenBatch<Vertex> Mesh;
typedef glt::GenBatch<Vertex> CubeMesh;

#define FREEZE_MESH(m) m.send()
#define CUBE_MESH(m) UNUSED(m)
#define ADD_VERT(e, v) e.add(v)

#else

DEFINE_VERTEX_ATTRS(vertexAttrs, Vertex,
    VERTEX_ATTR(Vertex, position),
    VERTEX_ATTR(Vertex, normal)
);

typedef glt::Mesh<Vertex> Mesh;
typedef glt::CubeMesh<Vertex> CubeMesh;

#define FREEZE_MESH(e) e.send()
#define CUBE_MESH(e) UNUSED(e)
#define ADD_VERT(e, v) e.addVertexElem(v)

#endif

void makeSphere(Mesh& sphere, float rad, int32 stacks, int32 slices);

void makeUnitCube(CubeMesh& cube);

std::ostream& LOCAL operator << (std::ostream& out, const vec3_t& v) {
    return out << "(" << v.x << ";" << v.y << ";" << v.z << ")";
}

} // namespace anon

struct Game EXPLICIT : public ge::GameWindow {

    CubeMesh wallBatch;
    Mesh sphereBatches[SPHERE_LOD_MAX];
    Mesh lineBatch;
    CubeMesh rectBatch;

    World world;
    float sphere_speed;
    Sphere sphere_proto;
    Renderer renderer;
    
    glt::ShaderManager shaderManager;
    glt::RenderManager renderManager;
    glt::GeometryTransform& transformPipeline;

    glt::ShaderProgram wallShader;
    struct {
        vec3_t ecLightPos;
        vec3_t ecSpotDir;
        float spotAngle;
    } wallUniforms;
    
    glt::ShaderProgram sphereShader;
    struct {
        vec3_t ecLightPos;
        vec3_t ecSpotDir;
        float spotAngle;
    } sphereUniforms;

    glt::ShaderProgram postprocShader;
    
    glt::ShaderProgram sphereInstancedShader;

    glt::ShaderProgram identityShader;

    glt::TextureRenderTarget *textureRenderTarget;

    float game_speed;

    bool use_interpolation;
    bool render_spheres_instanced;
    bool indirect_rendering;

    std::vector<SphereInstance> sphere_instances[SPHERE_LOD_MAX];

    Game();

    void updateIndirectRendering(bool indirect);
    void resizeRenderTargets();

    sf::ContextSettings createContextSettings() OVERRIDE;
    bool onInit() OVERRIDE;

    void animate() OVERRIDE;
    void renderScene(float interpolation) OVERRIDE;
    void renderWorld(float dt);
    
    
    void keyStateChanged(const sf::Event::KeyEvent& key, bool pressed) OVERRIDE;
    void mouseButtonStateChanged(const sf::Event::MouseButtonEvent& button, bool pressed) OVERRIDE;
    void windowResized(uint32 width, uint32 height) OVERRIDE;
    void mouseMoved(int32 dx, int32 dy) OVERRIDE;
    void handleInternalEvents() OVERRIDE;
    void spawn_sphere();

    SphereLOD calc_sphere_lod(const Sphere& s);
    void render_sphere(const Sphere& s, const SphereModel& m);
    void end_render_spheres();
    void render_box(const glt::AABB& box);
    void render_con(const point3_t& a, const point3_t& b);
    void render_hud();
    
    bool load_shaders();
    void update_sphere_mass();
};

Game::Game() :
    wallBatch(vertexAttrs),
    lineBatch(vertexAttrs),
    rectBatch(vertexAttrs),
    renderer(*this),
    transformPipeline(renderManager.geometryTransform()),
    wallShader(shaderManager),
    sphereShader(shaderManager),
    postprocShader(shaderManager),
    sphereInstancedShader(shaderManager),
    identityShader(shaderManager)
{}

sf::ContextSettings Game::createContextSettings() {
    sf::ContextSettings cs;
    cs.MajorVersion = 3;
    cs.MinorVersion = 3;
    cs.DepthBits = 24;
    cs.StencilBits = 0;
    cs.CoreProfile = false;
#ifdef GLDEBUG
    cs.DebugContext = true;
#endif
    return cs;
}

bool Game::onInit() {

    configureShaderVersion(shaderManager);

    GLuint vao;
    GL_CHECK(glGenVertexArrays(1, &vao));
    GL_CHECK(glBindVertexArray(vao));
    
    textureRenderTarget = 0;
    indirect_rendering = false;
    updateIndirectRendering(indirect_rendering);

    render_spheres_instanced = true;

    CUBE_MESH(wallBatch);

#ifdef GLDEBUG
    shaderManager.verbosity(glt::ShaderManager::Info);
#else
    shaderManager.verbosity(glt::ShaderManager::Quiet);
#endif

    {
        CUBE_MESH(rectBatch);

        Vertex v;
        v.normal = vec3(0.f, 0.f, 1.f);
        v.position = vec4(-1.f, -1.f, 0.f, 1.f); rectBatch.add(v);
        v.position = vec4( 1.f, -1.f, 0.f, 1.f); rectBatch.add(v);
        v.position = vec4( 1.f,  1.f, 0.f, 1.f); rectBatch.add(v);
        v.position = vec4(-1.f,  1.f, 0.f, 1.f); rectBatch.add(v);

        FREEZE_MESH(rectBatch);
    }

    shaderManager.addPath("shaders");

    ticksPerSecond(100);
    synchronizeDrawing(true);
    
    use_interpolation = true;
    grabMouse(true);

    if (!world.init())
        return false;

    world.render_by_distance = true;

    sphere_speed = 10.f;
    sphere_proto.state = Bouncing;
    sphere_proto.m = 1.f;
    sphere_proto.r = 0.3f;

    game_speed = 1.f;
    
    makeUnitCube(wallBatch);

    struct {
        int32 a, b;
    } sphere_params[SPHERE_LOD_MAX] = {
        { 12, 6 }, { 16, 8 },
        { 20, 10 }, { 24, 12 },
        { 26, 13 }, { 36, 18 }
    };

    for (uint32 i = 0; i < SPHERE_LOD_MAX; ++i) {
        sphereBatches[i].init(vertexAttrs);
        makeSphere(sphereBatches[i], 1.f, sphere_params[i].a, sphere_params[i].b);
    }

    world.camera().setOrigin(vec3(-19.f, 10.f, +19.f));
    world.camera().lookingAt(vec3(0.f, 10.f, 0.f));

    if (!load_shaders()) {
        std::cerr << "couldnt load shaders!" << std::endl;
        return false;
    }

    update_sphere_mass();

    windowResized(window().GetWidth(), window().GetHeight());

    Vertex vert;
    vert.normal = vec3(1.f, 0.f, 0.f);
    vert.position = vec4(vec3(0.f), 1.f);
    ADD_VERT(lineBatch, vert);
    vert.position = vec4(1.f, 0.f, 0.f, 1.f);
    ADD_VERT(lineBatch, vert);
    FREEZE_MESH(lineBatch);

    return true;
}

bool Game::load_shaders() {

    const struct {
        std::string basename;
        glt::ShaderProgram& prog;
    } vertexProgs[] = {
        { "brick", wallShader },
        { "sphere", sphereShader },
        { "identity", identityShader },
        { "postproc", postprocShader },
        { "sphere_instanced", sphereInstancedShader }
    };

    bool ok = true;
    for (uint32 i = 0; i < ARRAY_LENGTH(vertexProgs); ++i) {
        glt::ShaderProgram prog(shaderManager);
        prog.addShaderFilePair(vertexProgs[i].basename);
        prog.bindAttribute("vertex", vertexAttrs.index(offsetof(Vertex, position)));
        prog.bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
        prog.tryLink();
        ok = ok && vertexProgs[i].prog.replaceWith(prog);
    }

    return ok;
}

void Game::animate() {
    float dt = game_speed * frameDuration();
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

void Game::windowResized(uint32 width, uint32 height) {

    ASSERT(renderTarget().viewport() == glt::Viewport());
    ASSERT(renderTarget().width() == width);
    ASSERT(renderTarget().height() == height);
    
    std::cerr << "new window dimensions: " << width << "x" << height << std::endl;

    resizeRenderTargets();
    
    float fov = degToRad(17.5f);
    float aspect = float(width) / float(height);
    renderManager.setPerspectiveProjection(fov, aspect, PROJ_Z_MIN, PROJ_Z_MAX);
}

void Game::keyStateChanged(const sf::Event::KeyEvent& key, bool pressed) {

    if (!pressed) return;

    using namespace sf::Key;

    switch (key.Code) {
    case I: use_interpolation = !use_interpolation; break;
    case B: pause(!paused()); break;
    case C: load_shaders(); break;
    case G: if (world.solve_iterations > 0) --world.solve_iterations; goto print_iter;
    case H: ++world.solve_iterations; goto print_iter;
    case M:
        world.render_by_distance = !world.render_by_distance;
        std::cerr << "render_by_distance: " << (world.render_by_distance ? "yes" : "no") << std::endl;
        break;
    case N:
        render_spheres_instanced = !render_spheres_instanced;
        std::cerr << "Instanced Rendering: " << (render_spheres_instanced ? "yes" : "no") << std::endl;
        break;
    case V:
        updateIndirectRendering(!indirect_rendering);
        std::cerr << "Indirect Rendering: " << (indirect_rendering ? "yes" : "no") << std::endl;
        break;
    }

    return;

print_iter:
    std::cerr << "number of contact-solver iterations: " << world.solve_iterations << std::endl;    
}

void Game::mouseMoved(int32 dx, int32 dy) {
//    std::cerr << "mouse moved: " << dx << ", " << dy << std::endl;
    world.rotateCamera(dx * 0.001f, dy * 0.001f);
}

void Game::mouseButtonStateChanged(const sf::Event::MouseButtonEvent& e, bool pressed) {
    if (!pressed) return;
    
    if (e.Button == sf::Mouse::Left)
        spawn_sphere();
}

void Game::handleInternalEvents() {
    using namespace sf::Key;

    float z_step = 0.f;
    if (isKeyDown(W))
        z_step = +1.f;
    else if (isKeyDown(S))
        z_step = -1.f;

    float x_step = 0.f;
    if (isKeyDown(A))
        x_step = 1.f;
    else if (isKeyDown(D))
        x_step = -1.f;

    if (z_step != 0 || x_step != 0) 
        world.moveCamera(normalize(vec3(x_step, 0.f, z_step)) * CAMERA_STEP, CAMERA_SPHERE_RAD);

    if (isKeyDown(R))
        sphere_proto.r += 0.05f;
    else if (isKeyDown(E))
        sphere_proto.r = std::max(0.05f, sphere_proto.r - 0.05f);

    update_sphere_mass();
    
    if (isKeyDown(P))
        sphere_speed += 0.25f;
    else if (isKeyDown(O))
        sphere_speed = std::max(0.25f, sphere_speed - 0.25f);

    if (isKeyDown(U))
        game_speed += 0.025f;
    else if (isKeyDown(Z))
        game_speed = std::max(0.f, game_speed - 0.025f);
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
    vec3_t direction = world.camera().localZ();
    sphere_proto.center = world.camera().origin + direction * (sphere_proto.r + 1.1f);
    sphere_proto.v = direction * sphere_speed;

    SphereModel model;
    model.color = randomColor();
    model.shininess = 10.f + rand1() * 60;

    world.spawnSphere(sphere_proto, model);
}

void Game::renderScene(float interpolation) {

    if (!use_interpolation)
        interpolation = 0.f;

    const mat4_t camMat = transformationWorldToLocal(world.camera());
    renderManager.setCameraMatrix(camMat);

    renderManager.beginScene();
    renderManager.activeRenderTarget()->clear(glt::RT_DEPTH_BUFFER);
    GL_CHECK(glEnable(GL_DEPTH_TEST));

    window().SetActive();

    float dt = interpolation * game_speed * frameDuration();

    renderWorld(dt);

    if (indirect_rendering) {
        GL_CHECK(glDisable(GL_DEPTH_TEST));
        
        renderManager.setActiveRenderTarget(&this->renderTarget());
        postprocShader.use();
        
        textureRenderTarget->textureHandle().bind(0);
        glt::Uniforms(postprocShader)
            .optional("gammaCorrection", GAMMA)
            .mandatory("textures", textureRenderTarget->textureHandle(), 0);
        
        rectBatch.draw();
        
    }

    render_hud();
        
    renderManager.endScene();

    if (indirect_rendering) 
        renderManager.setActiveRenderTarget(0);
}

void Game::renderWorld(float dt) {
    const vec3_t eyeLightPos = transformPipeline.transformPoint(LIGHT_POS);
    const vec3_t spotDirection = normalize(vec3(0.f) - LIGHT_POS);
    const vec3_t ecSpotDirection = transformPipeline.transformVector(spotDirection);
        
    wallUniforms.ecLightPos = eyeLightPos;
    wallUniforms.ecSpotDir = ecSpotDirection;
    wallUniforms.spotAngle = 0.91;

    sphereUniforms.ecLightPos = eyeLightPos;
    sphereUniforms.ecSpotDir = ecSpotDirection;
    sphereUniforms.spotAngle = 0.91;
        
    world.render(renderer, dt);
}

const glt::ViewFrustum& Renderer::frustum() {
    return game.renderManager.viewFrustum();
}

void Game::updateIndirectRendering(bool indirect) {
    glt::RenderTarget *rt;

    if (indirect) {

        if (textureRenderTarget == 0)
            textureRenderTarget = new glt::TextureRenderTarget(window().GetWidth(), window().GetHeight(), glt::RT_COLOR_BUFFER | glt::RT_DEPTH_BUFFER);
        
        rt = textureRenderTarget;
    } else {
        rt = &this->renderTarget();
    }

    indirect_rendering = indirect;
    
    renderManager.setActiveRenderTarget(0);
    resizeRenderTargets();
    renderManager.setDefaultRenderTarget(rt);
}

void Game::resizeRenderTargets() {
    uint32 width = window().GetWidth();
    uint32 height = window().GetHeight();
    
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

            sphereInstancedShader.use();

            sphereMap.bind(0);

            glt::Uniforms(sphereInstancedShader)
                .optional("normalMatrix", transformPipeline.normalMatrix())
                .optional("vMatrix", transformPipeline.mvMatrix())
                .optional("pMatrix", transformPipeline.projectionMatrix())
                .optional("ecLight", sphereUniforms.ecLightPos)
                .optional("gammaCorrection", indirect_rendering ? 1.f : GAMMA)
                .mandatory("instanceData", sphereMap, 0);

            sphereBatches[lod].drawInstanced(num);

            sphereMap.free();
        }
    }
}

SphereLOD Game::calc_sphere_lod(const Sphere& s) {
    vec3_t ecCoord = transformPipeline.transformPoint(s.center);

    // near upper right corner of frustm (view coord)
    vec4_t nur_corner = transformPipeline.inverseProjectionMatrix() * vec4(-1.f, -1.f, -1.f, 1.f);
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
        const vec3_t& pos = s.center;
        glt::SavePoint sp(transformPipeline.save());
        transformPipeline.translate(vec3(pos.x, pos.y, pos.z));
        transformPipeline.scale(vec3(s.r));

        sphereShader.use();

        vec3_t col3 = vec3(1.f);
        col3 /= lod.level + 1;
        glt::color col = glt::color(col3);
        
        glt::Uniforms us(sphereShader);
        us.optional("mvpMatrix", transformPipeline.mvpMatrix());
        us.optional("mvMatrix", transformPipeline.mvMatrix());
        us.optional("normalMatrix", transformPipeline.normalMatrix());
        us.optional("ecLight", sphereUniforms.ecLightPos);
        us.optional("ecSpotDirection", sphereUniforms.ecSpotDir);
        us.optional("spotAngle", sphereUniforms.spotAngle);
        us.optional("color", col);
        us.optional("shininess", m.shininess);
        us.optional("gammaCorrection", indirect_rendering ? 1.f : GAMMA);
        
#ifdef GLDEBUG
        sphereShader.validate();
#endif
        sphereBatches[lod.level].draw();
    }
}

void Renderer::renderBox(const glt::AABB& box) {
    game.render_box(box);
}

void Game::render_box(const glt::AABB& box) {

    GL_CHECK(glCullFace(GL_FRONT));

    glt::SavePoint sp(transformPipeline.save());

    point3_t center = box.center();
    vec3_t diff = 0.5f * box.dimensions();

    transformPipeline.translate(vec3(center.x, center.y, center.z));
    transformPipeline.scale(vec3(diff.x, diff.y, diff.z));

    wallShader.use();
    
    glt::Uniforms us(wallShader);
    us.optional("mvpMatrix", transformPipeline.mvpMatrix());
    us.optional("mvMatrix", transformPipeline.mvMatrix());
    us.optional("normalMatrix", transformPipeline.normalMatrix());
    us.optional("ecLight", wallUniforms.ecLightPos);
    us.optional("ecSpotDirection", wallUniforms.ecSpotDir);
    us.optional("spotAngle", wallUniforms.spotAngle);
    us.optional("gammaCorrection", indirect_rendering ? 1.f : GAMMA);

#ifdef GLDEBUG    
    wallShader.validate(true);
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

    glt::SavePoint sp(transformPipeline.save());

    transformPipeline.translate(a);
    mat3_t trans = mat3(b - a,
                        vec3(0.f, 1.f, 0.f),
                        vec3(0.f, 0.f, 1.f));
    transformPipeline.concat(trans);

    identityShader.use();
    glt::Uniforms us(identityShader);
    us.optional("mvpMatrix", transformPipeline.mvpMatrix());
    us.optional("color", CONNECTION_COLOR);
    us.optional("gammaCorrection", indirect_rendering ? 1.f : GAMMA);

    // std::cerr << "con: a = " << va << ", b = " << vb << std::endl
    //           << "  a' = " << transformPipeline.transformPoint(vec3(0.f))
    //           << ", b' = " << transformPipeline.transformPoint(vec3(1.f, 0.f, 0.f))
    //           << std::endl;

    lineBatch.draw();

    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
}

void Game::render_hud() {

    const glt::FrameStatistics& fs = renderManager.frameStatistics();
    
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

    window().SaveGLStates();
    
    window().Draw(txtFps);
    window().Draw(txtSpeed);
    window().Draw(txtMass);
    window().Draw(txtRad);
    window().Draw(txtAnimSpeed);
    window().Draw(txtInter);
    window().Draw(txtNumBalls);
    window().Draw(txtRenderTime);
    
    window().RestoreGLStates();
}

int main(int argc, char *argv[]) {
    UNUSED(argc);

    if (argv[0] != 0)
        fs::cwdBasenameOf(argv[0]);

    Game game;
    if (!game.init("sim-sfml")) {
        return 1;
    }

    return game.run();
}


namespace {

void makeUnitCube(CubeMesh& cube) {
    Vertex v;
    
    v.normal = vec3(0.f, 0.f, -1.f);
    v.position = vec4(-1.0f, -1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f, -1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f,  1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f,  1.0f,  1.0f, 1.f); cube.add(v);

    v.normal = vec3(0.f, 0.f, +1.f);
    v.position = vec4(-1.0f, -1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f,  1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f,  1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f, -1.0f, -1.0f, 1.f); cube.add(v);

    v.normal = vec3(0.f, -1.f, 0.f);
    v.position = vec4(-1.0f,  1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f,  1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f,  1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f,  1.0f, -1.0f, 1.f); cube.add(v);

    v.normal = vec3(0.f, +1.f, 0.f);
    v.position = vec4(-1.0f, -1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f, -1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f, -1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f, -1.0f,  1.0f, 1.f); cube.add(v);

    v.normal = vec3(-1.f, 0.f, 0.f);
    v.position = vec4( 1.0f, -1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f,  1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f,  1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4( 1.0f, -1.0f,  1.0f, 1.f); cube.add(v);

    v.normal = vec3(+1.f, 0.f, 0.f);
    v.position = vec4(-1.0f, -1.0f, -1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f, -1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f,  1.0f,  1.0f, 1.f); cube.add(v);
    v.position = vec4(-1.0f,  1.0f, -1.0f, 1.f); cube.add(v);

    FREEZE_MESH(cube);
}

void addTriangle(Mesh& s, const vec3_t vertices[3], const vec3_t normals[3], const vec2_t texCoords[3]) {

    UNUSED(texCoords);
    
    for (uint32 i = 0; i < 3; ++i) {
        Vertex v;
        v.position = vec4(vertices[i], 1.f);
        v.normal = normals[i];
        ADD_VERT(s, v);
    }
}

// from the GLTools library (OpenGL Superbible)
void gltMakeSphere(Mesh& sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks)
{
    GLfloat drho = (GLfloat)(3.141592653589) / (GLfloat) iStacks;
    GLfloat dtheta = 2.0f * (GLfloat)(3.141592653589) / (GLfloat) iSlices;
    GLfloat ds = 1.0f / (GLfloat) iSlices;
    GLfloat dt = 1.0f / (GLfloat) iStacks;
    GLfloat t = 1.0f;	
    GLfloat s = 0.0f;
    GLint i, j;     // Looping variables
    
    for (i = 0; i < iStacks; i++) 
    {
        GLfloat rho = (GLfloat)i * drho;
        GLfloat srho = (GLfloat)(sin(rho));
        GLfloat crho = (GLfloat)(cos(rho));
        GLfloat srhodrho = (GLfloat)(sin(rho + drho));
        GLfloat crhodrho = (GLfloat)(cos(rho + drho));
		
        // Many sources of OpenGL sphere drawing code uses a triangle fan
        // for the caps of the sphere. This however introduces texturing 
        // artifacts at the poles on some OpenGL implementations
        s = 0.0f;
        vec3_t vVertex[4];
        vec3_t vNormal[4];
        vec2_t vTexture[4];

        for ( j = 0; j < iSlices; j++) 
        {
            GLfloat theta = (j == iSlices) ? 0.0f : j * dtheta;
            GLfloat stheta = (GLfloat)(-sin(theta));
            GLfloat ctheta = (GLfloat)(cos(theta));
			
            GLfloat x = stheta * srho;
            GLfloat y = ctheta * srho;
            GLfloat z = crho;
        
            vTexture[0][0] = s;
            vTexture[0][1] = t;
            vNormal[0][0] = x;
            vNormal[0][1] = y;
            vNormal[0][2] = z;
            vVertex[0][0] = x * fRadius;
            vVertex[0][1] = y * fRadius;
            vVertex[0][2] = z * fRadius;
			
            x = stheta * srhodrho;
            y = ctheta * srhodrho;
            z = crhodrho;

            vTexture[1][0] = s;
            vTexture[1][1] = t - dt;
            vNormal[1][0] = x;
            vNormal[1][1] = y;
            vNormal[1][2] = z;
            vVertex[1][0] = x * fRadius;
            vVertex[1][1] = y * fRadius;
            vVertex[1][2] = z * fRadius;
			

            theta = ((j+1) == iSlices) ? 0.0f : (j+1) * dtheta;
            stheta = (GLfloat)(-sin(theta));
            ctheta = (GLfloat)(cos(theta));
			
            x = stheta * srho;
            y = ctheta * srho;
            z = crho;
        
            s += ds;
            vTexture[2][0] = s;
            vTexture[2][1] = t;
            vNormal[2][0] = x;
            vNormal[2][1] = y;
            vNormal[2][2] = z;
            vVertex[2][0] = x * fRadius;
            vVertex[2][1] = y * fRadius;
            vVertex[2][2] = z * fRadius;
			
            x = stheta * srhodrho;
            y = ctheta * srhodrho;
            z = crhodrho;

            vTexture[3][0] = s;
            vTexture[3][1] = t - dt;
            vNormal[3][0] = x;
            vNormal[3][1] = y;
            vNormal[3][2] = z;
            vVertex[3][0] = x * fRadius;
            vVertex[3][1] = y * fRadius;
            vVertex[3][2] = z * fRadius;

            addTriangle(sphereBatch, vVertex, vNormal, vTexture);
			
            // Rearrange for next triangle
            vVertex[0] = vVertex[1];
            vNormal[0] = vNormal[1];
            vTexture[0] = vTexture[1];
			
            vVertex[1] = vVertex[3];
            vNormal[1] = vNormal[3];
            vTexture[1] = vTexture[3];
					
            addTriangle(sphereBatch, vVertex, vNormal, vTexture);			
        }
        t -= dt;
    }
    
    FREEZE_MESH(sphereBatch);
}

void makeSphere(Mesh& sphere, float rad, int32 stacks, int32 slices) {
    gltMakeSphere(sphere, rad, stacks, slices);
}

} // namespace anon
