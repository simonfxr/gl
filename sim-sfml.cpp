#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <climits>

#define GLEW_STATIC

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLFrame.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "math/vec3.hpp"
#include "math/ivec3.hpp"
#include "math/mat4.hpp"
#include "math/mat3.hpp"
#include "math/math.hpp"
#include "math/transform.hpp"

#include "glt/utils.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/Uniforms.hpp"
#include "glt/color.hpp"
#include "glt/GenBatch.hpp"
#include "glt/Frame.hpp"

#include "ge/GameWindow.hpp"

using namespace math;

static const vec3_t gravity = vec3(0.f, -9.81f, 0.f);

struct Game;

namespace {

struct Sphere;

struct Cuboid {
    vec3_t corner_min;
    vec3_t corner_max;
    bool touchesWall(const Sphere& s, vec3_t& out_normal, vec3_t& out_collision) const;
};

static const float sphere_density = 999;

struct Sphere {
    vec3_t vel;
    float r;
    point3_t center;
    float mass;
    glt::color color;
    float shininess;

    void move(const Game& game, float dt);
    point3_t calc_position(float dt) const;

    static bool collide(Sphere& x, Sphere& y, float dt);

    static vec3_t velocity_after_collision(float m1, const vec3_t& v1, float m2, const vec3_t& v2);

} __attribute__((aligned(16)));

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

struct Mirror {
    
    point3_t origin;
    float width;
    float height;

    uint32 pix_width;
    uint32 pix_height;
    
    GLuint fbo_name;
    GLuint depth_buffer_name;
    GLuint texture;

    bool rendering_recursive;

    glt::GenBatch<Vertex> batch;
    glt::ShaderProgram shader;

    Mirror(glt::ShaderManager& sm);

    bool init(Game& game);
    void resized(Game& game);
    void createTexture(Game& game, float dt);
    void render(Game& game);

    static const bool DISABLE;
};

const bool Mirror::DISABLE = true;

Mirror::Mirror(glt::ShaderManager& sm) :
    batch(GL_QUADS, vertexAttrs),
    shader(sm)
{}

static const uint32 BLUR_BUFFERS = 6;

struct Blur {

    static const bool DISABLE;
    
    GLuint textures[BLUR_BUFFERS];
    GLuint pix_buffer;
    
    uint32 head, size;

    uint32 pix_width, pix_height;

    byte *pixel_data;

    glt::ShaderProgram shader;

    bool init(Game& game);
    void resized(Game& game);
    void render(Game& game, float dt);

    Blur(glt::ShaderManager& sm) : shader(sm) {}
};

const bool Blur::DISABLE = true;

struct Camera {
    glt::Frame frame;

    // vec3_t origin;
    // EulerAngles orientation;

    void setOrigin(const vec3_t& origin);
    void facePoint(const vec3_t& position);
    void rotate(float rotx, float roty);

    vec3_t getOrigin();
    vec3_t getForwardVector();

    void move_along_local_axis(const vec3_t& step);
    void move_forward(float step);
    void move_right(float step);

    mat4_t getCameraMatrix();
        
} __attribute__ ((aligned(16)));

void set_matrix(M3DMatrix44f dst, const mat4_t& src) {
    for (uint32 i = 0; i < 16; ++i)
        dst[i] = src.components[i];
}

std::ostream& LOCAL operator << (std::ostream& out, const vec3_t& v) {
    return out << "(" << v.x << ";" << v.y << ";" << v.z << ")";
}

} // namespace anon

static const float FPS_RENDER_CYCLE = 1.f;

struct Game : public ge::GameWindow {

    GLMatrixStack           modelViewMatrix;            // Modelview Matrix
    GLMatrixStack           projectionMatrix;           // Projection Matrix
    GLFrustum               viewFrustum;                // View Frustum
    GLGeometryTransform     transformPipeline;          // Geometry Transform Pipeline

    glt::GenBatch<Vertex> wallBatch;
    glt::GenBatch<Vertex> sphereBatch;
    
    Camera camera;
    Cuboid room;
    Mirror mirror;
    Blur blur;                 

    glt::ShaderManager shaderManager;

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

    GLuint default_framebuffer;
    GLenum default_drawbuffer;

    uint32 fps_count;
    uint32 current_fps;
    float  fps_render_next;
    float  fps_render_last;

    float sphere_speed;
    float sphere_mass;
    float sphere_rad;

    float game_speed;

    bool use_interpolation;
    uint64 last_frame_rendered;

    bool sort_by_distance;

    std::vector<Sphere> spheres;

    Game(sf::RenderWindow& win, sf::Clock& clock);

    bool touchesWall(const Sphere& s, vec3_t& out_normal, vec3_t& out_collision) const;

    bool onInit();

    void animate();
    void renderScene(float interpolation);
    void renderWorld(float dt);
    
    void keyStateChanged(sf::Key::Code key, bool pressed);
    void mouseButtonStateChanged(sf::Mouse::Button button, bool pressed);
    void windowResized(uint32 width, uint32 height);
    void mouseMoved(int32 dx, int32 dy);
    void handleInternalEvents();

    void spawn_sphere();
    void move_camera(const vec3_t& step);

    void renderSpheres(float dt);
    void render_hud();
    void render_walls();

    void restoreDefaultBuffers();
    
    void resolve_collisions(float dt);
    bool load_shaders();
    void update_sphere_mass();
};

Game::Game(sf::RenderWindow& win, sf::Clock& clock) :
    ge::GameWindow(win, clock),
    wallBatch(GL_QUADS, vertexAttrs),
    sphereBatch(GL_TRIANGLES, vertexAttrs),
    mirror(shaderManager),
    blur(shaderManager),
    wallShader(shaderManager),
    sphereShader(shaderManager)
{}

static void print_context(const sf::ContextSettings& c) {
    
    std::cerr << "Initialized OpenGL Context"<< std::endl
              << "  Version:\t" << c.MajorVersion << "." << c.MinorVersion << std::endl
              << "  DepthBits:\t" << c.DepthBits << std::endl
              << "  StencilBits:\t" << c.StencilBits << std::endl
              << "  Antialiasing:\t" << c.AntialiasingLevel << std::endl
              << "  DebugContext:\t" << (c.DebugContext ? "yes" : "no") << std::endl
              << std::endl;
}

bool Game::onInit() {

    print_context(window().GetSettings());

    window().SetActive();

    default_framebuffer = 0;
    default_drawbuffer = GL_BACK_LEFT;

    window().UseVerticalSync(false);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        return false;
    }

    if (window().GetSettings().DebugContext)
        glt::initDebug();

#ifdef GLDEBUG
    shaderManager.verbosity(glt::ShaderManager::Info);
#else
    shaderManager.verbosity(glt::ShaderManager::Quiet);
#endif

    shaderManager.addPath(".");
    shaderManager.addPath("shaders");

    if (!mirror.init(*this))
        return false;

    if (!blur.init(*this))
        return false;

    ticksPerSecond(100);
    grabMouse(true);

    sort_by_distance = true;

    last_frame_rendered = 0;

    sphere_speed = 10.f;
    sphere_mass = 1.f;
    sphere_rad = 0.3f;

    game_speed = 1.f;
    use_interpolation = true;
    
    room.corner_min = vec3(-20.f, 0, -20.f);
    room.corner_max = vec3(+20.f, 20.f, 20.f);

    makeUnitCube(wallBatch);

    GL_CHECK(glEnable(GL_DEPTH_TEST));
    
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

    makeSphere(sphereBatch, 1.f, 26, 13);

    camera.setOrigin(vec3(-19.f, 10.f, +19.f));
    camera.facePoint(vec3(0.f, 10.f, 0.f));

    if (!load_shaders()) {
        std::cerr << "couldnt load shaders!" << std::endl;
        return false;
    }

    fps_count = 0;
    fps_render_next = fps_render_last = 0.f;

    update_sphere_mass();
    
    windowResized(window().GetWidth(), window().GetHeight());

    return true;
}

bool Mirror::init(Game& game) {
    rendering_recursive = false;

    glt::GenBatch<Vertex>& q = batch;
    Vertex v;
    v.normal = vec3(0.f, 0.f, 1.f);
    
    v.position = vec4(0.f, 0.f, 0.f, 1.f); q.add(v);
    v.position = vec4(1.f, 0.f, 0.f, 1.f); q.add(v);
    v.position = vec4(1.f, 1.f, 0.f, 1.f); q.add(v);
    v.position = vec4(0.f, 1.f, 0.f, 1.f); q.add(v);

    q.freeze();

    origin = vec3(1, 1, 0);
    width = 5.f;
    height = 5.f;

    pix_width = 1200;
    pix_height = 1200;

    if (DISABLE) return true;

    GL_CHECK(glGenFramebuffers(1, &fbo_name));
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_name));

    GL_CHECK(glGenRenderbuffers(1, &depth_buffer_name));
    GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_name));
    GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, pix_width, pix_height));
    GL_CHECK(glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_name));

    GL_CHECK(glGenTextures(1, &texture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pix_width, pix_height, 0, GL_RGBA, GL_FLOAT, NULL));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GL_CHECK(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0));


    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, game.default_framebuffer));

    return true;
}

bool Blur::init(Game& game) {

    if (DISABLE) return true;

    pix_width = game.window().GetWidth() + 1;
    pix_height = game.window().GetHeight() + 1;

    memset(textures, 0, BLUR_BUFFERS * sizeof textures[0]);
    pix_buffer = 0;
    pixel_data = 0;

    resized(game);

    return true;
}

void Blur::resized(Game& game) {

    if (DISABLE) return;

    if (pix_width == game.window().GetWidth() &&
        pix_height == game.window().GetHeight())
        return;
    
    pix_width = game.window().GetWidth();
    pix_height = game.window().GetHeight();
    
    head = size = 0;

    delete[] pixel_data;

    GL_CHECK(glDeleteTextures(BLUR_BUFFERS, textures));
    GL_CHECK(glDeleteBuffers(1, &pix_buffer));

    GL_CHECK(glGenTextures(BLUR_BUFFERS, textures));

    uint32 image_size = pix_width * pix_height * 4;
    pixel_data = new byte[image_size];
    memset(pixel_data, 0, image_size);

    for (uint32 i = 0; i < BLUR_BUFFERS; ++i) {
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + i));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, textures[i]));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pix_width, pix_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
    }

    GL_CHECK(glGenBuffers(1, &pix_buffer));
    GL_CHECK(glBindBuffer(GL_PIXEL_PACK_BUFFER, pix_buffer));
    GL_CHECK(glBufferData(GL_PIXEL_PACK_BUFFER, image_size, pixel_data, GL_DYNAMIC_COPY));
    GL_CHECK(glBindBuffer(GL_PIXEL_PACK_BUFFER, 0));
}

void Game::restoreDefaultBuffers() {
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, default_framebuffer));
    GL_CHECK(glDrawBuffers(1, &default_drawbuffer));
    GL_CHECK(glViewport(0, 0, window().GetWidth(), window().GetHeight()));    
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

    if (!Mirror::DISABLE) {

        glt::ShaderProgram ms(shaderManager);
        ms.addShaderFile(glt::ShaderProgram::VertexShader, "shaders/mirror.vert");
        ms.addShaderFile(glt::ShaderProgram::FragmentShader, "shaders/mirror.frag");
        ms.bindAttribute("position", vertexAttrs.index(offsetof(Vertex, position)));
        ms.bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
        ms.tryLink();

        ok = ok && !ms.wasError();

        if (!ms.wasError())
            mirror.shader.replaceWith(ms);
        else
            ms.printError(std::cerr);
    }

    if (!Blur::DISABLE) {
        glt::ShaderProgram bs(shaderManager);
        
        bs.addShaderFile(glt::ShaderProgram::VertexShader, "shaders/blur.vert");
        bs.addShaderFile(glt::ShaderProgram::FragmentShader, "shaders/blur.frag");
        bs.bindAttribute("vertex", vertexAttrs.index(offsetof(Vertex, position)));
        bs.bindAttribute("normal", vertexAttrs.index(offsetof(Vertex, normal)));
        bs.tryLink();
        
        ok = ok && !bs.wasError();
        
        if (!bs.wasError())
            blur.shader.replaceWith(bs);
        else
            bs.printError(std::cerr);
    }

    return ok;
}

void Game::animate() {
    float dt  = game_speed * frameDuration();

    resolve_collisions(dt);

    for (uint32 i = 0; i < spheres.size(); ++i)
        spheres[i].move(*this, dt);
}

// namespace {

// static const uint32 ALLOC_BLOCK_SIZE = 1024;
// static const uint32 DIM = 50;

// struct SphereCollisionHandler {

//     struct Tile {
//         uint32 id;
//         Tile *nxt;
//     };

//     struct AllocBlock {
//         AllocBlock *nxt;
//         Tile block[ALLOC_BLOCK_SIZE];

//         AllocBlock(AllocBlock *pred = 0) :
//             nxt(pred)
//             {}
//     };

//     Tile *tiles[DIM][DIM][DIM];
    
//     std::vector<uint32> last_collisions;
//     uint32 next_tile;
//     AllocBlock *blocks;

//     SphereCollisionHandler(uint32 nspheres) :
//         last_collisions(nspheres, 0),
//         next_tile(0),
//         blocks(new AllocBlock)
//     {
//         memset(tiles, 0, DIM * DIM * DIM * sizeof tiles[0][0][0]);
//     }

//     ~SphereCollisionHandler() {
//         while (blocks != 0) {
//             AllocBlock *b = blocks;
//             blocks = blocks->nxt;
//             delete b;
//         }            
//     }

//     void insertTile(uint32 x, uint32 y, uint32 z, uint32 id) {
        
//         if (unlikely(next_tile >= ALLOC_BLOCK_SIZE)) {
//             next_tile = 0;
//             blocks = new AllocBlock(blocks);
//         }

//         Tile *t = &blocks->block[next_tile++];

//         t->id = id;
//         t->nxt = tiles[x][y][z];
//         tiles[x][y][z] = t;
//     }

//     void collTile(std::vector<Sphere>& ss, float dt, uint32 x, uint32 y, uint32 z, uint32 id) {
//         Sphere &s = ss[id];
//         Tile *t = tiles[x][y][z];
        
//         while (t != 0) {
            
//             if (t->id < id && last_collisions[id] <= t->id) {
//                 Sphere::collide(s, ss[t->id], dt);
//                 last_collisions[id] = t->id + 1;
//             }
            
//             t = t->nxt;
//         }
//     }
    
// };

// } // namespace anon

// static ivec3 calcTile(const vec3_t& isize, const vec3_t& room_min, const vec3_t& p) {
//     return ivec3(compMult(isize, p - room_min)); 
// }

// static void calcRange(const Sphere& s, float dt, const vec3_t& isize, const vec3_t& room_min, ivec3& tileL, ivec3& tileH) {

//     vec3_t pos1 = s.center;
//     vec3_t pos2 = s.calc_position(dt);

//     vec3_t cLow  = min(pos1, pos2) - vec3(s.r * 1.1f);
//     vec3_t cHigh = max(pos1, pos2) + vec3(s.r * 1.1f);

//     tileL = calcTile(isize, room_min, cLow);
//     tileH = calcTile(isize, room_min, cHigh);
    
// }

// static ivec3 clamp(const ivec3& low, const ivec3& high, const ivec3& v) {
//     return imax(low, imin(high, v));
// }

// static void clampRange(const ivec3& low, const ivec3& high, ivec3& range_low, ivec3& range_high) {
//     range_low = clamp(low, high, range_low);
//     range_high = clamp(low, high, range_high);
// }

void Game::resolve_collisions(float dt) {

    // float t0 = now();

    // {
    //     const uint32 dim = 50;
    //     const vec3_t diff = room.corner_max - room.corner_min;
    //     const vec3_t isize = vec3(dim / diff.x, dim / diff.y, dim / diff.z);
    //     const vec3_t low  = room.corner_min;

    //     const ivec3 tmin = ivec3(0);
    //     const ivec3 tmax = ivec3(dim - 1);

    //     SphereCollisionHandler coll(spheres.size());

    //     for (uint32 i = 0; i < spheres.size(); ++i) {
        
    //         Sphere& s = spheres[i];

    //         ivec3 tileL, tileH;
    //         calcRange(s, dt, isize, room.corner_min, tileL, tileH);
    //         clampRange(tmin, tmax, tileL, tileH);

    //         for (int32 x = tileL.x; x <= tileH.x; ++x)
    //             for (int32 y = tileL.y; y <= tileH.y; ++y)
    //                 for (int32 z = tileL.z; z <= tileH.z; ++z)
    //                     coll.insertTile(x, y, z, i);
    //     }

    //     for (unsigned i = 0; i < spheres.size(); ++i) {
        
    //         Sphere& s = spheres[i];
    //         ivec3 tileL, tileH;
    //         calcRange(s, dt, isize, room.corner_min, tileL, tileH);
    //         clampRange(tmin, tmax, tileL, tileH);

    //         for (int32 x = tileL.x; x <= tileH.x; ++x)
    //             for (int32 y = tileL.y; y <= tileH.y; ++y)
    //                 for (int32 z = tileL.z; z <= tileH.z; ++z)
    //                     coll.collTile(spheres, dt, x, y, z, i);
    //     }
    // }

    // std::cerr << "resolve_collisions: " << (now() - t0) << " seconds" << std::endl;

    for (uint32 i = 0; i < spheres.size(); ++i)
        for (uint32 j = 0; j < i; ++j)
            Sphere::collide(spheres[i], spheres[j], dt);
    
}

template <typename T>
static std::string to_string(T x) {
    std::stringstream out;
    out << x;
    return out.str();
}

void Game::windowResized(uint32 width, uint32 height) {

    std::cerr << "new window dimensions: " << width << "x" << height << std::endl;
    GL_CHECK(glViewport(0, 0, width, height));

    // Create the projection matrix, and load it on the projection matrix stack
    viewFrustum.SetPerspective(35.0f, float(width) / float(height), 1.0f, 100.0f);
    // M3DMatrix44f projMat;
    // set_matrix(projMat, Transform::perspective(math::degToRad(35.0f), float(width) / float(height), 1.0f, 100.0f));
    
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
//    projectionMatrix.LoadMatrix(projMat);
        
    // Set the transformation pipeline to use the two matrix stacks 
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

    mirror.resized(*this);
    
    blur.resized(*this);
}

void Game::keyStateChanged(sf::Key::Code key, bool pressed) {

    if (!pressed) return;

    using namespace sf::Key;

    switch (key) {
    case I: use_interpolation = !use_interpolation; break;
    case B: pause(!paused()); break;
    case C: load_shaders(); break;
    case M:
        sort_by_distance = !sort_by_distance;
        std::cerr << "sort_by_distance: " << (sort_by_distance ? "yes" : "no") << std::endl;
        break;
    }

    update_sphere_mass();
}

void Game::mouseMoved(int32 dx, int32 dy) {

    // M3DMatrix44f rot;
    // camera.frame.GetMatrix(rot, true);
    // EulerAngles orientation(toMat4(rot));

    // camera.orientation.heading -= dx * 0.001f;
    // camera.orientation.pitch -= dy * 0.001f;

    // if (camera.orientation.pitch < -0.5f * math::PI)
    //     camera.orientation.pitch = -0.5f * math::PI;
    // else if (camera.orientation.pitch > 0.5f * math::PI)
    //     camera.orientation.pitch = 0.5f * math::PI;

    // camera.orientation.canonize();

    // camera.frame.RotateLocalY(dx * 0.001f);
    // camera.frame.RotateLocalX(dy * 0.001f);

    camera.rotate(dx * 0.001f, dy * 0.001f);
}

void Game::mouseButtonStateChanged(sf::Mouse::Button button, bool pressed) {

    if (!pressed) return;
    
    if (button == sf::Mouse::Left)
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
        move_camera(vec3(x_step, 0.f, z_step));
    
    if (isKeyDown(R))
        sphere_rad += 0.05f;
    else if (isKeyDown(E))
        sphere_rad = std::max(0.05f, sphere_rad - 0.05f);
    
    if (isKeyDown(P))
        sphere_speed += 0.25f;
    else if (isKeyDown(O))
        sphere_speed = std::max(0.25f, sphere_speed - 0.25f);

    if (isKeyDown(U))
        game_speed += 0.025f;
    else if (isKeyDown(Z))
        game_speed = std::max(0.f, game_speed - 0.025f);
}

void Game::move_camera(const vec3_t& dir) {

    const float STEP = 0.1f;
    vec3_t step = normalize(dir) * STEP;

//    std::cerr << "moving camera: " << currentFrameID() << ", " << dir << std::endl;

    Camera new_cam = camera;
    new_cam.move_along_local_axis(step);
    
    Sphere cam;
    cam.center = new_cam.getOrigin();
    cam.r = 1.f;

    vec3_t norm;
    vec3_t col;
    if (!touchesWall(cam, norm, col))
        camera = new_cam;
}

void Game::update_sphere_mass() {
    float V = 4.f / 3.f * math::PI * math::cubed(sphere_rad);
    sphere_mass = V * sphere_density;
}

static const GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f};
static const vec4_t SPHERE_COLOR = vec4(0.f, 0.f, 1.f, 1.f);
static const M3DVector4f vLightPos = { 11.f, 18.f, 11.f, 1.f };
static const M3DVector4f vWallColor = { 0.6f, 0.6f, 0.6f, 1.f };

static float rand1() {
    return rand() * (1.f / RAND_MAX);
}

static glt::color randomColor() {
    return glt::color(byte(rand1() * 255), byte(rand1() * 255), byte(rand1() * 255));
}

void Game::spawn_sphere() {

    vec3_t direction = camera.getForwardVector();

    Sphere s;
    s.center = camera.getOrigin() + direction * (sphere_rad + 1.1f);
    s.vel = direction * sphere_speed;
    s.r = sphere_rad;
    s.mass = sphere_mass;
    s.color = randomColor();
    s.shininess = 10.f + rand1() * 60;

    spheres.push_back(s);
}

void Game::renderScene(float interpolation) {

    if (!use_interpolation && last_frame_rendered == currentFrameID())
        return;

    if (!use_interpolation)
        interpolation = 0.f;
    
    last_frame_rendered = currentFrameID();
    
    window().SetActive();

    float dt = interpolation * game_speed * frameDuration();

    glEnable(GL_CULL_FACE);
    
    renderWorld(dt);

    glDisable(GL_CULL_FACE);

    blur.render(*this, dt);

    ++fps_count;
    render_hud();
}

static GLFrame asGLFrame(const glt::Frame& ref) {
    GLFrame fr;

    point3_t pos = ref.getOrigin();
    fr.SetOrigin(pos.x, pos.y, pos.z);
    direction3_t fw = ref.localZ();
    fr.SetForwardVector(fw.x, fw.y, fw.z);
    direction3_t up = ref.localY();
    fr.SetUpVector(up.x, up.y, up.z);

    return fr;
}

void Game::renderWorld(float dt) {

    GLFrame fr = asGLFrame(camera.frame);
    viewFrustum.Transform(fr);

    mirror.createTexture(*this, dt);

    fr = asGLFrame(camera.frame);
    viewFrustum.Transform(fr);

    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    modelViewMatrix.PushMatrix();

    M3DMatrix44f mCamera;
    set_matrix(mCamera, camera.getCameraMatrix());
    modelViewMatrix.PushMatrix(mCamera);

    M3DVector4f vLightEyePos;
    m3dTransformVector4(vLightEyePos, vLightPos, mCamera);
    const vec3_t eyeLightPos = vec3(vLightEyePos[0], vLightEyePos[1], vLightEyePos[2]);

    const vec3_t& spotDirection = normalize(vec3(0.f) - vec3(vLightPos));
    const M3DVector4f vSpotDirection = { spotDirection.x, spotDirection.y, spotDirection.z, 0 };

    M3DVector4f vEyeSpotDirection;
    m3dTransformVector4(vEyeSpotDirection, vSpotDirection, mCamera);
    const vec3_t ecSpotDirection = vec3(vEyeSpotDirection);

    wallUniforms.ecLightPos = eyeLightPos;
    wallUniforms.ecSpotDir = ecSpotDirection;
    wallUniforms.spotAngle = 0.91;

    sphereUniforms.ecLightPos = eyeLightPos;
    sphereUniforms.ecSpotDir = ecSpotDirection;
    sphereUniforms.spotAngle = 0.91;

    render_walls();

    renderSpheres(dt);

    mirror.render(*this);

    modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();    
}

void Mirror::createTexture(Game& game, float dt) {

    if (DISABLE) return;

    if (rendering_recursive)
        return;

    const vec3_t corners[] = { origin,
                               origin + vec3(width, 0.f, 0.f),
                               origin + vec3(width, height, 0.f),
                               origin + vec3(0.f, height, 0.f) };

    bool visible = false;

    for (uint32 i = 0; i < 4 && !visible; ++i)
        visible = game.viewFrustum.TestSphere(corners[i].x, corners[i].y, corners[i].z, 0.f);

    if (!visible)
        return;

    Camera old_cam = game.camera;

    GLFrustum old_frust = game.viewFrustum;
    
    game.viewFrustum.SetPerspective(35.0f, width / height, 1.0f, 100.0f);

    static const vec3_t mirror_normal = vec3(0.f, 0.f, 1.f);

    const vec3_t mirror_center = 0.5f * (corners[0] + corners[2]);

    const vec3_t fw = game.camera.getForwardVector();

    vec3_t mirror_view = normalize(reflect(fw, mirror_normal));

    game.camera.frame.setOrigin(mirror_center - 2.f * mirror_view);
    game.camera.frame.setYZ(vec3(0.f, 1.f, 0.f), vec3(mirror_view.x, 0.f, mirror_view.z));

    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_name));

    static const GLenum fboBuffs[] = { GL_COLOR_ATTACHMENT0 };
    
    GL_CHECK(glDrawBuffers(1, fboBuffs));
    GL_CHECK(glViewport(0, 0, pix_width, pix_height));

    game.modelViewMatrix.PushMatrix();
    game.modelViewMatrix.Scale(-1.f, 1.f, 1.f);

    rendering_recursive = true;

    game.renderWorld(dt);

    rendering_recursive = false;

    game.modelViewMatrix.PopMatrix();
    
    game.viewFrustum = old_frust;
    game.camera = old_cam;

    game.restoreDefaultBuffers();
}

void Mirror::render(Game& game) {

    if (DISABLE) return;

    if (rendering_recursive)
        return;

    game.modelViewMatrix.PushMatrix();
    game.modelViewMatrix.Translate(origin.x, origin.y, origin.z);
    game.modelViewMatrix.Scale(width, height, 1.f);

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));

    shader.use();
    
    glt::Uniforms us(shader);
    
    us.optional("mvpMatrix", mat4(game.transformPipeline.GetModelViewProjectionMatrix()));

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));

    GLint texPos;
    GL_CHECK(texPos = glGetUniformLocation(shader.program(), "mirrorTexture"));
    GL_CHECK(glUniform1i(texPos, 0));
    
    batch.draw();

    game.modelViewMatrix.PopMatrix();
}

void Mirror::resized(Game& game) {
    UNUSED(game);
}

void Blur::render(Game& game, float dt) {

    static float next_update = 0.f;

    UNUSED(dt);

    if (DISABLE) return;

    if (game.realTime() >= next_update) {
        next_update = game.realTime() + 1;
        

        GL_CHECK(glBindBuffer(GL_PIXEL_PACK_BUFFER, pix_buffer));
        GL_CHECK(glReadPixels(0, 0, pix_width, pix_height, GL_RGB, GL_UNSIGNED_BYTE, NULL));
        GL_CHECK(glBindBuffer(GL_PIXEL_PACK_BUFFER, 0));
        
        GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pix_buffer));
        
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + head));
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, pix_width, pix_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
        GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
    }

    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
//    GL_CHECK(glDisable(GL_DEPTH_TEST));

    shader.use();

    for (uint32 i = 0; i < BLUR_BUFFERS; ++i) {
        uint32 k = (head + i) % BLUR_BUFFERS;

        std::string tex_name = "texture" + to_string(i);

        GL_CHECK(glActiveTexture(GL_TEXTURE0 + i));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, textures[k]));
        GLint loc;
        GL_CHECK(loc = glGetUniformLocation(shader.program(), tex_name.c_str()));
        if (loc == -1)
            ERROR("foo");
        GL_CHECK(glUniform1i(loc, i));
    }


    game.mirror.batch.draw();
        
//    GL_CHECK(glEnable(GL_DEPTH_TEST));

    head = (head + 1) % BLUR_BUFFERS;
    size = min(BLUR_BUFFERS, size + 1);
}

struct SphereModel {
    float viewDist;
    vec3_t pos;
    const Sphere *s;

    static bool compare(const SphereModel& m1, const SphereModel& m2) {
        return m1.viewDist < m2.viewDist;
    }
};

void Game::renderSpheres(float dt) {

    sphereShader.use();

    const vec3_t camPos = camera.getOrigin();
 
    std::vector<SphereModel> toRender;

    for (uint32 i = 0; i < spheres.size(); ++i) {
        const Sphere& s = spheres[i];

        const vec3_t pos = s.calc_position(dt);

        if (!viewFrustum.TestSphere(pos.x, pos.y, pos.z, s.r))
            continue;

        SphereModel m;
        m.s = &s;
        m.pos = pos;
        m.viewDist = std::max(0.f, distance(pos, camPos) - s.r);
        toRender.push_back(m);
    }

    std::sort(toRender.begin(), toRender.end(), SphereModel::compare);

    if (!sort_by_distance)
        std::reverse(toRender.begin(), toRender.end());

    for (uint32 i = 0; i < toRender.size(); ++i) {
        const Sphere& s = *toRender[i].s;
        const vec3_t& pos = toRender[i].pos;
        
        modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(pos.x, pos.y, pos.z);
        modelViewMatrix.Scale(s.r, s.r, s.r);

        glt::Uniforms us(sphereShader);
        us.optional("mvpMatrix", mat4(transformPipeline.GetModelViewProjectionMatrix()));
        us.optional("mvMatrix", mat4(transformPipeline.GetModelViewMatrix()));
        us.optional("normalMatrix", mat3(transformPipeline.GetNormalMatrix(true)));
        us.optional("ecLight", sphereUniforms.ecLightPos);
        us.optional("ecSpotDirection", sphereUniforms.ecSpotDir);
        us.optional("spotAngle", sphereUniforms.spotAngle);
        us.optional("color", s.color);
        us.optional("shininess", s.shininess);

#ifdef GLDEBUG
        sphereShader.validate();
#endif

        sphereBatch.draw();
        
        modelViewMatrix.PopMatrix();
    }
}

void Game::render_walls() {

    GL_CHECK(glCullFace(GL_FRONT));

    modelViewMatrix.PushMatrix();

    vec3_t center = 0.5f * (room.corner_max + room.corner_min);
    vec3_t diff = 0.5f * (room.corner_max - room.corner_min);

    modelViewMatrix.Translate(center.x, center.y, center.z);
    modelViewMatrix.Scale(diff.x, diff.y, diff.z);

    wallShader.use();
    
    glt::Uniforms us(wallShader);
    us.optional("mvpMatrix", mat4(transformPipeline.GetModelViewProjectionMatrix()));
    us.optional("mvMatrix", mat4(transformPipeline.GetModelViewMatrix()));
    us.optional("normalMatrix", mat3(transformPipeline.GetNormalMatrix(true)));
    us.optional("ecLight", wallUniforms.ecLightPos);
    us.optional("ecSpotDirection", wallUniforms.ecSpotDir);
    us.optional("spotAngle", wallUniforms.spotAngle);

    wallShader.validate(GLDEBUG_ENABLED);
    wallBatch.draw();

    modelViewMatrix.PopMatrix();
    
    GL_CHECK(glCullFace(GL_BACK));    
}

void Game::render_hud() {

    float now = realTime();
    if (now >= fps_render_next) {

        current_fps = fps_count / (now - fps_render_last);
        fps_render_next = now + FPS_RENDER_CYCLE;
        fps_render_last = now;
        fps_count = 0;
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
    sf::Text txtMass("Sphere mass: " + to_string(sphere_mass) + " kg");
    txtMass.SetCharacterSize(16);
    txtMass.SetColor(c);
    txtMass.SetPosition(2, height);

    height += txtMass.GetRect().Height + 2;
    sf::Text txtRad("Sphere radius: " + to_string(sphere_rad) + " m");
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
    sf::Text txtNumBalls(std::string("NO Spheres: ") + to_string(spheres.size()));
    txtNumBalls.SetCharacterSize(16);
    txtNumBalls.SetColor(c);
    txtNumBalls.SetPosition(2, height);

    window().SaveGLStates();
    
    window().Draw(txtFps);
    window().Draw(txtSpeed);
    window().Draw(txtMass);
    window().Draw(txtRad);
    window().Draw(txtAnimSpeed);
    window().Draw(txtInter);
    window().Draw(txtNumBalls);
    
    window().RestoreGLStates();
}

int main(int argc, char *argv[]) {

    UNUSED(argc);
    sf::Clock startupTimer;
    float startupBegin = startupTimer.GetElapsedTime();
    
    if (argv[0] != 0)
        gltSetWorkingDirectory(argv[0]);
    
    sf::ContextSettings glContext;
    glContext.MajorVersion = 3;
    glContext.MinorVersion = 3;
    glContext.AntialiasingLevel = 4;
#ifdef GLDEBUG
    glContext.DebugContext = true;
#endif

    sf::RenderWindow win(sf::VideoMode(800, 600), "sim-sfml", sf::Style::Default, glContext);

    Game game(win, startupTimer);

    if (!game.init(glContext, "sim-sfml")) {
        std::cerr << "initialization failed, exiting..." << std::endl;
        return 1;
    }

    float startupTime = startupTimer.GetElapsedTime() - startupBegin;
    std::cerr << "initialized in " << uint32(startupTime * 1000) << " ms." << std::endl;

    return game.run();
}

void Sphere::move(const Game& game, float dt) {
    center = calc_position(dt);
    vel += gravity * dt;

    vec3_t wall;
    vec3_t collision;
    
    if (game.touchesWall(*this, wall, collision)) {        
        vel = reflect(vel, wall, rand1() * 0.1f + 0.9f) * 0.8f;
        center = collision + wall * -r;
    }
}

vec3_t Sphere::calc_position(float dt) const {
    return center + vel * dt + 0.5f * dt * dt * gravity;
}

vec3_t Sphere::velocity_after_collision(float m1, const vec3_t& v1, float m2, const vec3_t& v2) {
    return (m1 * v1 + m2 * (2.f * v2 - v1)) / (m1 + m2);
}

bool Sphere::collide(Sphere& x, Sphere& y, float dt) {

    vec3_t xc = x.calc_position(dt);
    vec3_t yc = y.calc_position(dt);

    float r = x.r + y.r;
    
    if (distanceSq(xc, yc) < r*r) {

        // HACK: if we collided in the last frame and collision wasnt resolved correctly
        // do it now, with a crude approximation (well no approx. at all...)
        if (distanceSq(x.center, y.center) < r*r) {

            vec3_t n1 = normalize(y.center - x.center);

            x.vel = length(x.vel) * -n1;
            y.vel = length(y.vel) * n1;

        } else {

            direction3_t toY = directionFromTo(xc, yc);
            direction3_t toX = -toY;
            
            vec3_t vx_coll = projectAlong(x.vel, toY);
            vec3_t vx_rest = x.vel - vx_coll;

            vec3_t vy_coll = projectAlong(y.vel, toX);
            vec3_t vy_rest = y.vel - vy_coll;

            vec3_t ux = velocity_after_collision(x.mass, vx_coll, y.mass, vy_coll);
            // conservation of momentum
            vec3_t uy = (x.mass / y.mass) * (vx_coll - ux) + vy_coll;

            x.vel = (rand1() * 0.1f + 0.9f) * ux + vx_rest;
            y.vel = (rand1() * 0.1f + 0.9f) * uy + vy_rest;
        }

        return true;
    }

    return false;
}

void Camera::move_along_local_axis(const vec3_t& step) {
    frame.translateLocal(step);
}

void Camera::move_forward(float step) {
    move_along_local_axis(vec3(0, 0, 1) * step);
}

void Camera::move_right(float step) {
    move_along_local_axis(vec3(1, 0, 0) * step);
}

vec3_t Camera::getOrigin() {
    return frame.getOrigin();
}

vec3_t Camera::getForwardVector() {
    return frame.localZ();
}

void Camera::setOrigin(const vec3_t& orig) {
    frame.setOrigin(orig);
}

void Camera::facePoint(const vec3_t& pos) {
    direction3_t z = directionFromTo(getOrigin(), pos);
    frame.setXZ(normalize(cross(frame.localY(), z)), z);
}

mat4_t Camera::getCameraMatrix() {
    return frame.cameraMatrix();
}

void Camera::rotate(float rotx, float roty) {
    frame.rotateLocal(roty, vec3(1.f, 0.f, 0.f));
    frame.rotateWorld(rotx, vec3(0.f, 1.f, 0.f));
}

bool Cuboid::touchesWall(const Sphere& s, vec3_t& out_normal, vec3_t& out_collision) const {

    if (s.center.y - s.r < corner_min.y) {
        out_normal = vec3(0, -1.f, 0);
        out_collision = vec3(s.center.x, corner_min.y, s.center.z);
        return true;
    }

    if (s.center.y + s.r > corner_max.y) {
        out_normal = vec3(0, 1.f, 0);
        out_collision = vec3(s.center.x, corner_max.y, s.center.z);
        return true;
    }

    if (s.center.x - s.r < corner_min.x) {
        out_normal = vec3(-1.0f, 0, 0);
        out_collision = vec3(corner_min.x, s.center.y, s.center.z);
        return true;
    }

    if (s.center.x + s.r > corner_max.x) {
        out_normal = vec3(1.f, 0.f, 0.f);
        out_collision = vec3(corner_max.x, s.center.y, s.center.z);
        return true;
    }

    if (s.center.z - s.r < corner_min.z) {
        out_normal = vec3(0.f, 0, -1.0f);
        out_collision = vec3(s.center.x, s.center.y, corner_min.z);
        return true;
    }

    if (s.center.z + s.r > corner_max.z) {
        out_normal = vec3(0.f, 0.f, 1.f);
        out_collision = vec3(s.center.x, s.center.y, corner_max.z);
        return true;
    }

    return false;
}

bool Game::touchesWall(const Sphere& s, vec3_t& out_normal, vec3_t& out_collision) const {

    if (room.touchesWall(s, out_normal, out_collision))
        return true;

    float zdist;
    if (!Mirror::DISABLE &&
        s.center.x + s.r >= mirror.origin.x &&
        s.center.x - s.r <= mirror.origin.x + mirror.width &&
        s.center.y + s.r >= mirror.origin.y &&
        s.center.y - s.r <= mirror.origin.y + mirror.height &&
        math::abs(zdist = (s.center.z - mirror.origin.z)) < s.r) {

        out_normal = vec3(0.f, 0.f, -math::signum(zdist));
        out_collision = vec3(s.center.x, s.center.y, mirror.origin.z);
        return true;
    }

    return false;
}

namespace {

void makeUnitCube(glt::GenBatch<Vertex>& cube) {
    // cube.Begin(GL_QUADS, 24);

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

void addTriangle(glt::GenBatch<Vertex>& s, const M3DVector3f vertices[3], const M3DVector3f normals[3], const M3DVector2f texCoords[3]) {

    UNUSED(texCoords);
    
    for (uint32 i = 0; i < 3; ++i) {
        Vertex v;
        v.position = vec4(vec3(vertices[i]), 1.f);
        v.normal = vec3(normals[i]);
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
        M3DVector3f vVertex[4];
        M3DVector3f vNormal[4];
        M3DVector2f vTexture[4];

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
            memcpy(vVertex[0], vVertex[1], sizeof(M3DVector3f));
            memcpy(vNormal[0], vNormal[1], sizeof(M3DVector3f));
            memcpy(vTexture[0], vTexture[1], sizeof(M3DVector2f));
			
            memcpy(vVertex[1], vVertex[3], sizeof(M3DVector3f));
            memcpy(vNormal[1], vNormal[3], sizeof(M3DVector3f));
            memcpy(vTexture[1], vTexture[3], sizeof(M3DVector2f));
					
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
