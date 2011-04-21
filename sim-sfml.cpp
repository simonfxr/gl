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
#include "glt/GenBatch.hpp"
#include "glt/Frame.hpp"
#include "glt/GeometryTransform.hpp"
#include "glt/Transformations.hpp"
#include "glt/RenderManager.hpp"
#include "glt/TextureRenderTarget3D.hpp"

#include "ge/GameWindow.hpp"

#include "sim.hpp"

using namespace math;

static const float SPHERE_DENSITY = 999.f;

static const vec4_t SPHERE_COLOR = vec4(0.f, 0.f, 1.f, 1.f);

static const vec3_t LIGHT_POS = vec3(11.f, 18.f, 11.f);

static const float FPS_RENDER_CYCLE = 1.f;

static const float CAMERA_STEP = 0.1f;

static const float CAMERA_SPHERE_RAD = 1.f;

static const uint32 NUM_BLUR_TEXTURES = 1;

static const float BLUR_TEXTURES_UPDATE_CYCLE = 0.04f / NUM_BLUR_TEXTURES;

static const glt::color CONNECTION_COLOR(0x00, 0xFF, 0x00);

namespace {

typedef vec4_t SphereInstance; // pos.xyz; radius

struct Vertex {
    point4_t position;
    vec3_t normal;
};

glt::Attr vertexAttrArray[] = {
    glt::attr::vec4(offsetof(Vertex, position)),
    glt::attr::vec3(offsetof(Vertex, normal))
};

glt::Attrs<Vertex> vertexAttrs(ARRAY_LENGTH(vertexAttrArray), vertexAttrArray);

void makeSphere(glt::GenBatch<Vertex>& sphere, float rad, int32 stacks, int32 slices);

void makeUnitCube(glt::GenBatch<Vertex>& cube);

std::ostream& LOCAL operator << (std::ostream& out, const vec3_t& v) {
    return out << "(" << v.x << ";" << v.y << ";" << v.z << ")";
}

} // namespace anon

struct Game EXPLICIT : public ge::GameWindow {

    glt::GenBatch<Vertex> wallBatch;
    glt::GenBatch<Vertex> sphereBatch;
    glt::GenBatch<Vertex> lineBatch;
    glt::GenBatch<Vertex> rectBatch;

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

    glt::TextureRenderTarget3D *textureRenderTarget;
    
    uint32 nactive_texs;
    uint32 active_tex_idx;
    float next_tex_snapshot;

    uint32 fps_count;
    uint32 current_fps;
    float  fps_render_next;
    float  fps_render_last;

    float  current_avg_draw_time;
    uint32 num_draws;
    float  draw_time_accum;
    float  draw_time_render_next;

    float game_speed;

    bool use_interpolation;
    uint64 last_frame_rendered;

    bool render_spheres_instanced;

    std::vector<SphereInstance> sphere_instances;

    Game();

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
    sphereBatch(vertexAttrs),
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

bool Game::onInit() {

    textureRenderTarget = new glt::TextureRenderTarget3D(window().GetWidth(), window().GetHeight(), NUM_BLUR_TEXTURES,
                                                         glt::RT_COLOR_BUFFER | glt::RT_DEPTH_BUFFER);

    renderManager.setRenderTarget(*textureRenderTarget);

    render_spheres_instanced = true;
    
    wallBatch.primType(GL_QUADS);
    sphereBatch.primType(GL_TRIANGLES);

#ifdef GLDEBUG
    shaderManager.verbosity(glt::ShaderManager::Info);
#else
    shaderManager.verbosity(glt::ShaderManager::Quiet);
#endif

    {
        rectBatch.primType(GL_QUADS);

        Vertex v;
        v.normal = vec3(0.f, 0.f, 1.f);
        v.position = vec4(-1.f, -1.f, 0.f, 1.f); rectBatch.add(v);
        v.position = vec4( 1.f, -1.f, 0.f, 1.f); rectBatch.add(v);
        v.position = vec4( 1.f,  1.f, 0.f, 1.f); rectBatch.add(v);
        v.position = vec4(-1.f,  1.f, 0.f, 1.f); rectBatch.add(v);
        
        rectBatch.freeze();
    }

    nactive_texs = 0;
    active_tex_idx = 0;

    shaderManager.addPath(".");
    shaderManager.addPath("shaders");

    ticksPerSecond(100);
    maxFPS(0);
    use_interpolation = true;
    grabMouse(true);

    if (!world.init())
        return false;

    world.render_by_distance = true;

    last_frame_rendered = 0;

    num_draws = 0;
    draw_time_accum = 0.f;

    sphere_speed = 10.f;
    sphere_proto.state = Bouncing;
    sphere_proto.m = 1.f;
    sphere_proto.r = 0.3f;

    game_speed = 1.f;
    
    makeUnitCube(wallBatch);

    GL_CHECK(glEnable(GL_DEPTH_TEST));
    
    makeSphere(sphereBatch, 1.f, 26, 13);

    world.camera().setOrigin(vec3(-19.f, 10.f, +19.f));
    world.camera().lookingAt(vec3(0.f, 10.f, 0.f));

    if (!load_shaders()) {
        std::cerr << "couldnt load shaders!" << std::endl;
        return false;
    }

    fps_count = 0;
    fps_render_next = fps_render_last = 0.f;

    update_sphere_mass();
    
    windowResized(window().GetWidth(), window().GetHeight());

    Vertex vert;
    vert.normal = vec3(1.f, 0.f, 0.f);
    vert.position = vec4(vec3(0.f), 1.f);
    lineBatch.add(vert);
    vert.position = vec4(1.f, 0.f, 0.f, 1.f);
    lineBatch.add(vert);
    lineBatch.freeze();

    return true;
}

bool Game::load_shaders() {

    bool ok = true;
    glt::ShaderProgram ws(shaderManager);
    
    ws.addShaderFile(glt::ShaderProgram::VertexShader, "shaders/brick.vert");
    ws.addShaderFile(glt::ShaderProgram::FragmentShader, "shaders/brick.frag");
    ws.bindAttribute("vertex", vertexAttrs.index(offsetof(Vertex, position)));
    ws.bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
    ws.tryLink();

    ok = ok && !ws.wasError();
    
    if (!ws.wasError())
        wallShader.replaceWith(ws);
    else
        ws.printError(std::cerr);

    glt::ShaderProgram ss(shaderManager);

    ss.addShaderFile(glt::ShaderProgram::VertexShader, "shaders/sphere.vert");
    ss.addShaderFile(glt::ShaderProgram::FragmentShader, "shaders/sphere.frag");
    ss.bindAttribute("position", vertexAttrs.index(offsetof(Vertex, position)));
    ss.bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
    ss.tryLink();

    ok = ok && !ss.wasError();
    
    if (!ss.wasError())
        sphereShader.replaceWith(ss);
    else
        ss.printError(std::cerr);

    glt::ShaderProgram is(shaderManager);

    is.addShaderFile(glt::ShaderProgram::VertexShader, "shaders/identity.vert");
    is.addShaderFile(glt::ShaderProgram::FragmentShader, "shaders/identity.frag");
    is.bindAttribute("position", vertexAttrs.index(offsetof(Vertex, position)));
    is.bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
    is.tryLink();

    ok = ok && !is.wasError();
    
    if (!is.wasError())
        identityShader.replaceWith(is);
    else
        is.printError(std::cerr);

    glt::ShaderProgram ps(shaderManager);

    ps.addShaderFile(glt::ShaderProgram::VertexShader, "shaders/postproc.vert");
    ps.addShaderFile(glt::ShaderProgram::FragmentShader, "shaders/postproc.frag");
    ps.bindAttribute("position", vertexAttrs.index(offsetof(Vertex, position)));
    ps.bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
    ps.tryLink();

    ok = ok && !ps.wasError();
    
    if (!ps.wasError())
        postprocShader.replaceWith(ps);
    else
        ps.printError(std::cerr);

    glt::ShaderProgram sis(shaderManager);

    sis.addShaderFile(glt::ShaderProgram::VertexShader, "shaders/sphere_instanced.vert");
    sis.addShaderFile(glt::ShaderProgram::FragmentShader, "shaders/sphere_instanced.frag");
    sis.bindAttribute("position", vertexAttrs.index(offsetof(Vertex, position)));
    sis.bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
    sis.tryLink();

    ok = ok && !sis.wasError();
    
    if (!sis.wasError())
        sphereInstancedShader.replaceWith(sis);
    else
        sis.printError(std::cerr);

    return ok;
}

void Game::animate() {
    float dt  = game_speed * frameDuration();
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

    nactive_texs = 0;
    
    std::cerr << "new window dimensions: " << width << "x" << height << std::endl;
    this->renderTarget().viewport(glt::Viewport(width, height));
    textureRenderTarget->resize(width, height, NUM_BLUR_TEXTURES);
    textureRenderTarget->viewport(glt::Viewport(width, height));
    
    float fov = degToRad(17.5f);
    float aspect = float(width) / float(height);
    renderManager.setPerspectiveProjection(fov, aspect, 1.f, 100.f);

    next_tex_snapshot = realTime();
    textureRenderTarget->targetDepth(0);
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
    }

    return;

print_iter:
    std::cerr << "number of contact-solver iterations: " << world.solve_iterations << std::endl;    
}

void Game::mouseMoved(int32 dx, int32 dy) {
    std::cerr << "mouse moved: " << dx << ", " << dy << std::endl;
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

    // if (!use_interpolation && last_frame_rendered == currentFrameID())
    //     return;

    if (!use_interpolation)
        interpolation = 0.f;

    const mat4_t camMat = transformationWorldToLocal(world.camera());
    renderManager.setCameraMatrix(camMat);

    
    
    renderManager.beginScene();
    renderManager.renderTarget().clear(glt::RT_DEPTH_BUFFER);

    float t0 = now();
    
    last_frame_rendered = currentFrameID();
    
    window().SetActive();

    float dt = interpolation * game_speed * frameDuration();

    GL_CHECK(glEnable(GL_CULL_FACE));
    
    renderWorld(dt);

    GL_CHECK(glDisable(GL_CULL_FACE));

    ++fps_count;
    render_hud();

    ++num_draws;
    renderManager.endScene();
    renderManager.renderTarget().deactivate();

    bool inc = false;

    if (realTime() >= next_tex_snapshot) {
        
        if (nactive_texs + 1 < NUM_BLUR_TEXTURES)
            inc = true;

        textureRenderTarget->targetDepth((textureRenderTarget->targetDepth() + 1) % NUM_BLUR_TEXTURES);
        next_tex_snapshot = realTime() + BLUR_TEXTURES_UPDATE_CYCLE;
    }
                                 
    GL_CHECK(glDisable(GL_DEPTH_TEST));

    this->renderTarget().activate();
    postprocShader.use();

    textureRenderTarget->textureHandle().bind();
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GLint loc;
    GL_CHECK(loc = glGetUniformLocation(postprocShader.program(), "textures"));
    ASSERT(loc != -1);
    GL_CHECK(glUniform1i(loc, 0));
    glt::Uniforms(postprocShader)
        .optional("depth", float(nactive_texs + 1))
        .optional("offset", nactive_texs == 0 ? 2 : 1 / float(nactive_texs))
        .optional("gammaCorrection", 2.2);

    rectBatch.draw();
    
    this->renderTarget().draw();
    this->renderTarget().deactivate();

    GL_CHECK(glEnable(GL_DEPTH_TEST));

    if (inc)
        ++nactive_texs;

    draw_time_accum += now() - t0;
}

void Game::renderWorld(float dt) {
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

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

void Renderer::renderSphere(const Sphere& s, const SphereModel& m) {
    game.render_sphere(s, m);
}

void Renderer::endRenderSpheres() {
    game.end_render_spheres();
}

void Game::end_render_spheres() {
    if (render_spheres_instanced) {

        uint32 num = sphere_instances.size();

        if (num == 0)
            return;

        glt::TextureHandle sphereMap(glt::Texture1D);
        sphereMap.bind();

        GL_CHECK(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GL_CHECK(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        
        GL_CHECK(glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, num, 0, GL_RGBA, GL_FLOAT, &sphere_instances[0]));

        sphere_instances.clear();

        sphereInstancedShader.use();
        
        GL_CHECK(glActiveTexture(GL_TEXTURE0));
        sphereMap.bind();
        
        GLint loc;
        GL_CHECK(loc = glGetUniformLocation(sphereInstancedShader.program(), "instancesMap"));
        ASSERT(loc != -1);
        GL_CHECK(glUniform1i(loc, 0));

        glt::Uniforms(sphereInstancedShader)
            .optional("normalMatrix", transformPipeline.normalMatrix())
            .optional("vMatrix", transformPipeline.mvMatrix())
            .optional("pMatrix", transformPipeline.projectionMatrix());

        sphereBatch.drawInstanced(num);

        sphereMap.free();        
    }
}

void Game::render_sphere(const Sphere& s, const SphereModel& m) {

    if (render_spheres_instanced) {

        SphereInstance inst = vec4(s.center, s.r);
        sphere_instances.push_back(inst);
        
    } else {
        const vec3_t& pos = s.center;
        glt::SavePoint sp(transformPipeline.save());
        transformPipeline.translate(vec3(pos.x, pos.y, pos.z));
        transformPipeline.scale(vec3(s.r));
        
        sphereShader.use();
    
        glt::Uniforms us(sphereShader);
        us.optional("mvpMatrix", transformPipeline.mvpMatrix());
        us.optional("mvMatrix", transformPipeline.mvMatrix());
        us.optional("normalMatrix", transformPipeline.normalMatrix());
        us.optional("ecLight", sphereUniforms.ecLightPos);
        us.optional("ecSpotDirection", sphereUniforms.ecSpotDir);
        us.optional("spotAngle", sphereUniforms.spotAngle);
        us.optional("color", m.color);
        us.optional("shininess", m.shininess);
        
#ifdef GLDEBUG
        sphereShader.validate();
#endif
        sphereBatch.draw();
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

    // std::cerr << "con: a = " << va << ", b = " << vb << std::endl
    //           << "  a' = " << transformPipeline.transformPoint(vec3(0.f))
    //           << ", b' = " << transformPipeline.transformPoint(vec3(1.f, 0.f, 0.f))
    //           << std::endl;

    lineBatch.draw();

    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
}

void Game::render_hud() {
    float now = realTime();
    if (now >= fps_render_next) {
        current_fps = fps_count / (now - fps_render_last);
        fps_render_next = now + FPS_RENDER_CYCLE;
        fps_render_last = now;
        fps_count = 0;
    }

    if (now >= draw_time_render_next) {
        current_avg_draw_time = draw_time_accum / num_draws;
        num_draws = 0;
        draw_time_accum = 0.f;
        draw_time_render_next = now + FPS_RENDER_CYCLE;
    }

    sf::Color c(0, 255, 255, 180);

    uint32 height = 0;
    sf::Text txtFps = sf::Text(to_string(current_fps));
    txtFps.SetCharacterSize(16);
    txtFps.SetPosition(2, 0);
    txtFps.SetColor(sf::Color(255, 255, 0, 180));

    height += txtFps.GetRect().Height + 2;
    sf::Text txtSpeed("Sphere speed: " + to_string(sphere_speed) + " m/s");
    txtSpeed.SetCharacterSize(16);
    txtSpeed.SetColor(c);
    txtSpeed.SetPosition(2, height);

    height += txtSpeed.GetRect().Height + 2;    
    sf::Text txtMass("Sphere mass: " + to_string(sphere_proto.m) + " kg");
    txtMass.SetCharacterSize(16);
    txtMass.SetColor(c);
    txtMass.SetPosition(2, height);

    height += txtMass.GetRect().Height + 2;
    sf::Text txtRad("Sphere radius: " + to_string(sphere_proto.r) + " m");
    txtRad.SetCharacterSize(16);
    txtRad.SetColor(c);
    txtRad.SetPosition(2, height);

    height += txtRad.GetRect().Height + 2;
    sf::Text txtAnimSpeed("Animation Speed: " + to_string(game_speed));
    txtAnimSpeed.SetCharacterSize(16);
    txtAnimSpeed.SetColor(c);
    txtAnimSpeed.SetPosition(2, height);

    height += txtAnimSpeed.GetRect().Height + 2;
    sf::Text txtInter(std::string("Interpolation: ") + (use_interpolation ? "on" : "off"));
    txtInter.SetCharacterSize(16);
    txtInter.SetColor(c);
    txtInter.SetPosition(2, height);

    height += txtInter.GetRect().Height + 2;
    sf::Text txtNumBalls(std::string("NO Spheres: ") + to_string(world.numSpheres()));
    txtNumBalls.SetCharacterSize(16);
    txtNumBalls.SetColor(c);
    txtNumBalls.SetPosition(2, height);

    height += txtNumBalls.GetRect().Height + 2;
    sf::Text txtRenderTime(std::string("Render Time: ") + to_string(current_avg_draw_time * 1000) + " ms");
    txtRenderTime.SetCharacterSize(16);
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
        glt::setWorkingDirectory(argv[0]);

    Game game;
    if (!game.init("sim-sfml")) {
        return 1;
    }

    return game.run();
}


namespace {

void makeUnitCube(glt::GenBatch<Vertex>& cube) {
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

    cube.freeze();
}

void addTriangle(glt::GenBatch<Vertex>& s, const vec3_t vertices[3], const vec3_t normals[3], const vec2_t texCoords[3]) {

    UNUSED(texCoords);
    
    for (uint32 i = 0; i < 3; ++i) {
        Vertex v;
        v.position = vec4(vertices[i], 1.f);
        v.normal = normals[i];
        s.add(v);
    }
}

// from the GLTools library (OpenGL Superbible)
void gltMakeSphere(glt::GenBatch<Vertex>& sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks)
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
    
    sphereBatch.freeze();
}

void makeSphere(glt::GenBatch<Vertex>& sphere, float rad, int32 stacks, int32 slices) {
    gltMakeSphere(sphere, rad, stacks, slices);
}

} // namespace anon
