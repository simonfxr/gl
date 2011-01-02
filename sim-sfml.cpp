#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <limits.h>

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
#include "math/Math.hpp"
#include "math/EulerAngles.hpp"
#include "math/transform.hpp"

#include "GameWindow.hpp"
#include "gltools.hpp"
#include "ShaderProgram.hpp"
#include "GenBatch.hpp"
#include "color.hpp"
#include "Uniforms.hpp"

static const vec3 gravity(0.f, -9.81f, 0.f);

namespace {

struct Sphere;

struct Cuboid {
    vec3 corner_min;
    vec3 corner_max;
    bool touchesWall(const Sphere& s, vec3& out_normal, vec3& out_collision) const;
};

static const float sphere_density = 999;

struct Sphere {
    vec3 vel;
    float r;
    vec3 center;
    float mass;
    gltools::color color;
    float shininess;

    void move(const Cuboid& room, float dt);
    vec3 calc_position(float dt) const;

    static bool collide(Sphere& x, Sphere& y, float dt);

    static vec3 velocity_after_collision(float m1, const vec3& v1, float m2, const vec3& v2);

} __attribute__((aligned(16)));

struct CubeVertex {
    vec4 position;
    vec3 normal;
};

gltools::Attr cubeVertexAttrArray[] = {
    gltools::attr::vec4(offsetof(CubeVertex, position)),
    gltools::attr::vec3(offsetof(CubeVertex, normal))
};

gltools::Attrs<CubeVertex> cubeVertexAttrs(ARRAY_LENGTH(cubeVertexAttrArray), cubeVertexAttrArray);

struct SphereVertex {
    vec4 position;
    vec3 normal;
};

gltools::Attr sphereVertexAttrArray[] = {
    gltools::attr::vec4(offsetof(SphereVertex, position)),
    gltools::attr::vec3(offsetof(SphereVertex, normal)),
};

gltools::Attrs<SphereVertex> sphereVertexAttrs(ARRAY_LENGTH(sphereVertexAttrArray), sphereVertexAttrArray);

void makeSphere(gltools::GenBatch<SphereVertex>& sphere, float rad, int32 stacks, int32 slices);

void makeUnitCube(gltools::GenBatch<CubeVertex>& cube);

struct Camera {
    GLFrame frame;

    // vec3 origin;
    // EulerAngles orientation;

    void setOrigin(const vec3& origin);
    void facePoint(const vec3& position);
    void rotate(float rotx, float roty);

    vec3 getOrigin();
    vec3 getForwardVector();

    void move_along_local_axis(const vec3& step);
    void move_forward(float step);
    void move_right(float step);

    mat4 getCameraMatrix();
        
} __attribute__ ((aligned(16)));

void set_matrix(M3DMatrix44f dst, const mat4& src) {
    for (uint32 i = 0; i < 16; ++i)
        dst[i] = src.flat[i];
}

std::ostream& LOCAL operator << (std::ostream& out, const vec3& v) {
    return out << "(" << v.x << ";" << v.y << ";" << v.z << ")";
}

vec3 toVec3(const M3DVector3f v3) {
    return vec3(v3[0], v3[1], v3[2]);
}

mat4 toMat4(const M3DMatrix44f m) {
    mat4 M;
    for (uint32 i = 0; i < 16; ++i)
        M.flat[i] = m[i];
    return M;
}

mat3 toMat3(const M3DMatrix33f m) {
    mat3 M;
    for (uint32 i = 0; i < 9; ++i)
        M.flat[i] = m[i];
    return M;
}

} // namespace anon

static const float FPS_RENDER_CYCLE = 1.f;

struct Game : public GameWindow {

    GLMatrixStack           modelViewMatrix;            // Modelview Matrix
    GLMatrixStack           projectionMatrix;           // Projection Matrix
    GLFrustum               viewFrustum;                // View Frustum
    GLGeometryTransform     transformPipeline;          // Geometry Transform Pipeline

    gltools::GenBatch<CubeVertex> wallBatch;
    gltools::GenBatch<SphereVertex> sphereBatch;
    
    Camera                  camera;
    Cuboid                  room;

    ShaderProgram wallShader;
    struct {
        vec3 ecLightPos;
        vec3 ecSpotDir;
        float spotAngle;
    } wallUniforms;
    
    ShaderProgram sphereShader;
    struct {
        vec3 ecLightPos;
        vec3 ecSpotDir;
        float spotAngle;
    } sphereUniforms;

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

    std::vector<Sphere> spheres;

    Game();

private:

    bool onInit();

    void animate();
    void renderScene(float interpolation);
    
    void keyStateChanged(sf::Key::Code key, bool pressed);
    void mouseButtonStateChanged(sf::Mouse::Button button, bool pressed);
    void windowResized(uint32 width, uint32 height);
    void mouseMoved(int32 dx, int32 dy);
    void handleInternalEvents();

    void spawn_sphere();
    void move_camera(const vec3& step);

    void render_sphere(const Sphere& s, float dt);
    void render_hud();
    void render_walls();
    
    void resolve_collisions(float dt);
    bool load_shaders();
    void update_sphere_mass();
};

Game::Game() :
    wallBatch(GL_QUADS, cubeVertexAttrs),
    sphereBatch(GL_TRIANGLES, sphereVertexAttrs)
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

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        return false;
    }

    GL_CHECK((void) 0);
    
    // GLint nNumExtensions;
    // glGetIntegerv(GL_NUM_EXTENSIONS, &nNumExtensions);

    // std::cerr << "OpenGL extensions:" << std::endl;
    // for(GLint i = 0; i < nNumExtensions; i++) {
    //     const char *extension = (const char *) glGetStringi(GL_EXTENSIONS, i);
    //     std::cerr << extension << std::endl;
    // }
    // std::cerr << std::endl;

    ticksPerSecond(100);
    grabMouse(true);

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

    // This make a sphere
    makeSphere(sphereBatch, 1.f, 26, 13);

    const vec3 lln = room.corner_min;
    const vec3 dim = room.corner_max - room.corner_min;
    	
    // floorBatch.Begin(GL_LINES, 324);
    // for (GLfloat x = -20.0; x <= 20.0f; x+= 0.5) {
    //     floorBatch.Vertex3f(x, 0.f, 20.0f);
    //     floorBatch.Vertex3f(x, 0.f, -20.0f);
        
    //     floorBatch.Vertex3f(20.0f, 0.f, x);
    //     floorBatch.Vertex3f(-20.0f, 0.f, x);
    // }
    // floorBatch.End();

    // camera.origin = vec3(-20.f, 10.f, -20.f);
    // camera.orientation = EulerAngles(0, Math::PI, 0);
    // camera.orientation.canonize();

    // camera.setOrigin(room.corner_min);
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

bool Game::load_shaders() {

    bool ok = true;
    ShaderProgram ws;
    
    ws.addShaderFile(ShaderProgram::VertexShader, "brick.vert");
    ws.addShaderFile(ShaderProgram::FragmentShader, "brick.frag");
    ws.bindAttribute("vertex", cubeVertexAttrs.index(offsetof(CubeVertex, position)));
    ws.bindAttribute("normal", cubeVertexAttrs.index(offsetof(CubeVertex, normal)));
    ws.link();

    ok = ok && !ws.wasError();
    
    if (!ws.wasError())
        wallShader.replaceWith(ws);

    ws.printError(std::cerr);

    ShaderProgram ss;

    ss.addShaderFile(ShaderProgram::VertexShader, "sphere.vert");
    ss.addShaderFile(ShaderProgram::FragmentShader, "sphere.frag");
    ss.bindAttribute("position", sphereVertexAttrs.index(offsetof(SphereVertex, position)));
    ss.bindAttribute("normal", sphereVertexAttrs.index(offsetof(SphereVertex, normal)));
    ss.link();

    if (!ss.wasError())
        sphereShader.replaceWith(ss);

    ok = ok && !ss.wasError();
    
    ss.printError(std::cerr);

    return ok;
}

void Game::animate() {
    float dt  = game_speed * frameDuration();

    resolve_collisions(dt);

    for (uint32 i = 0; i < spheres.size(); ++i)
        spheres[i].move(room, dt);
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

// static ivec3 calcTile(const vec3& isize, const vec3& room_min, const vec3& p) {
//     return ivec3(vec3::compMult(isize, p - room_min)); 
// }

// static void calcRange(const Sphere& s, float dt, const vec3& isize, const vec3& room_min, ivec3& tileL, ivec3& tileH) {

//     vec3 pos1 = s.center;
//     vec3 pos2 = s.calc_position(dt);

//     vec3 cLow  = vec3::min(pos1, pos2) - vec3(s.r * 1.1f);
//     vec3 cHigh = vec3::max(pos1, pos2) + vec3(s.r * 1.1f);

//     tileL = calcTile(isize, room_min, cLow);
//     tileH = calcTile(isize, room_min, cHigh);
    
// }

// static ivec3 clamp(const ivec3& low, const ivec3& high, const ivec3& v) {
//     return ivec3::max(low, ivec3::min(high, v));
// }

// static void clampRange(const ivec3& low, const ivec3& high, ivec3& range_low, ivec3& range_high) {
//     range_low = clamp(low, high, range_low);
//     range_high = clamp(low, high, range_high);
// }

void Game::resolve_collisions(float dt) {

    // float t0 = now();

    // {
    //     const uint32 dim = 50;
    //     const vec3 diff = room.corner_max - room.corner_min;
    //     const vec3 isize = vec3(dim / diff.x, dim / diff.y, dim / diff.z);
    //     const vec3 low  = room.corner_min;

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
    // set_matrix(projMat, Transform::perspective(Math::degToRad(35.0f), float(width) / float(height), 1.0f, 100.0f));
    
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
//    projectionMatrix.LoadMatrix(projMat);
        
    // Set the transformation pipeline to use the two matrix stacks 
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

void Game::keyStateChanged(sf::Key::Code key, bool pressed) {

    if (!pressed) return;

    using namespace sf::Key;

    switch (key) {
    case I: use_interpolation = !use_interpolation; break;
    case B: pause(!paused()); break;
    case C: load_shaders(); break;
    }

    update_sphere_mass();
}

void Game::mouseMoved(int32 dx, int32 dy) {

    // M3DMatrix44f rot;
    // camera.frame.GetMatrix(rot, true);
    // EulerAngles orientation(toMat4(rot));

    // camera.orientation.heading -= dx * 0.001f;
    // camera.orientation.pitch -= dy * 0.001f;

    // if (camera.orientation.pitch < -0.5f * Math::PI)
    //     camera.orientation.pitch = -0.5f * Math::PI;
    // else if (camera.orientation.pitch > 0.5f * Math::PI)
    //     camera.orientation.pitch = 0.5f * Math::PI;

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

void Game::move_camera(const vec3& dir) {

    const float STEP = 0.1f;
    vec3 step = dir.normalize() * STEP;

//    std::cerr << "moving camera: " << currentFrameID() << ", " << dir << std::endl;

    Camera new_cam = camera;
    new_cam.move_along_local_axis(step);
    
    Sphere cam;
    cam.center = new_cam.getOrigin();
    cam.r = 1.f;

    vec3 norm;
    vec3 col;
    if (!room.touchesWall(cam, norm, col))
        camera = new_cam;
}

void Game::update_sphere_mass() {
    float V = 4.f / 3.f * Math::PI * Math::cubed(sphere_rad);
    sphere_mass = V * sphere_density;
}

static const GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f};
static const vec4 SPHERE_COLOR = vec4(0.f, 0.f, 1.f, 1.f);
static const M3DVector4f vLightPos = { 11.f, 18.f, 11.f, 1.f };
static const M3DVector4f vWallColor = { 0.6f, 0.6f, 0.6f, 1.f };

static float rand1() {
    return rand() * (1.f / RAND_MAX);
}

static gltools::color randomColor() {
    return gltools::color(byte(rand1() * 255), byte(rand1() * 255), byte(rand1() * 255));
}

void Game::spawn_sphere() {

    vec3 direction = camera.getForwardVector();

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

    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    modelViewMatrix.PushMatrix();

    M3DMatrix44f mCamera;
    set_matrix(mCamera, camera.getCameraMatrix());
    modelViewMatrix.PushMatrix(mCamera);

    M3DVector4f vLightEyePos;
    m3dTransformVector4(vLightEyePos, vLightPos, mCamera);
    const vec3 eyeLightPos = vec3(vLightEyePos[0], vLightEyePos[1], vLightEyePos[2]);

    const vec3& spotDirection = vec3::normalize(vec3(0) - toVec3(vLightPos));
    const M3DVector4f vSpotDirection = { spotDirection.x, spotDirection.y, spotDirection.z, 0 };

    M3DVector4f vEyeSpotDirection;
    m3dTransformVector4(vEyeSpotDirection, vSpotDirection, mCamera);
    const vec3 ecSpotDirection = toVec3(vEyeSpotDirection);

    wallUniforms.ecLightPos = eyeLightPos;
    wallUniforms.ecSpotDir = ecSpotDirection;
    wallUniforms.spotAngle = 0.91;

    sphereUniforms.ecLightPos = eyeLightPos;
    sphereUniforms.ecSpotDir = ecSpotDirection;
    sphereUniforms.spotAngle = 0.91;

    GL_CHECK(glCullFace(GL_FRONT));
    GL_CHECK(glEnable(GL_CULL_FACE));

    render_walls();

    float dt = interpolation * game_speed * frameDuration();

    GL_CHECK(glCullFace(GL_BACK));

    for (uint32 i = 0; i < spheres.size(); ++i)
        render_sphere(spheres[i], dt);

    GL_CHECK(glDisable(GL_CULL_FACE));

    modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();    

    ++fps_count;

    render_hud();

    gltools::printErrors(std::cerr);
}

void Game::render_sphere(const Sphere& s, float dt) {

    modelViewMatrix.PushMatrix();
    vec3 pos = s.calc_position(dt);
    modelViewMatrix.Translate(pos.x, pos.y, pos.z);
    modelViewMatrix.Scale(s.r, s.r, s.r);

    sphereShader.use();

    gltools::Uniforms us(sphereShader);
    us.optional("mvpMatrix", toMat4(transformPipeline.GetModelViewProjectionMatrix()));
    us.optional("mvMatrix", toMat4(transformPipeline.GetModelViewMatrix()));
    us.optional("normalMatrix", toMat3(transformPipeline.GetNormalMatrix(true)));
    us.optional("ecLight", sphereUniforms.ecLightPos);
    us.optional("ecSpotDirection", sphereUniforms.ecSpotDir);
    us.optional("spotAngle", sphereUniforms.spotAngle);
    us.optional("color", s.color);
    us.optional("shininess", s.shininess);
    
    sphereShader.validate(GLDEBUG_ENABLED);
    sphereBatch.draw();

    modelViewMatrix.PopMatrix();
}

void Game::render_walls() {

    modelViewMatrix.PushMatrix();

    vec3 center = 0.5f * (room.corner_max + room.corner_min);
    vec3 diff = 0.5f * (room.corner_max - room.corner_min);

    modelViewMatrix.Translate(center.x, center.y, center.z);
    modelViewMatrix.Scale(diff.x, diff.y, diff.z);

    wallShader.use();
    
    gltools::Uniforms us(wallShader);
    us.optional("mvpMatrix", toMat4(transformPipeline.GetModelViewProjectionMatrix()));
    us.optional("mvMatrix", toMat4(transformPipeline.GetModelViewMatrix()));
    us.optional("normalMatrix", toMat3(transformPipeline.GetNormalMatrix(true)));
    us.optional("ecLight", wallUniforms.ecLightPos);
    us.optional("ecSpotDirection", wallUniforms.ecSpotDir);
    us.optional("spotAngle", wallUniforms.spotAngle);

    wallShader.validate(GLDEBUG_ENABLED);
    wallBatch.draw();

    modelViewMatrix.PopMatrix();    
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

    Game game;

    if (!game.init(glContext, "sim-sfml")) {
        std::cerr << "initialization failed, exiting..." << std::endl;
        return 1;
    }

    float startupTime = startupTimer.GetElapsedTime() - startupBegin;
    std::cerr << "initialized in " << uint32(startupTime * 1000) << " ms." << std::endl;

    return game.run();
}

void Sphere::move(const Cuboid& room, float dt) {
    center = calc_position(dt);
    vel += gravity * dt;

    vec3 wall;
    vec3 collision;
    
    if (room.touchesWall(*this, wall, collision)) {
        vel = vec3::reflect(vel, wall, rand1() * 0.1f + 0.9f) * 0.8f;
        center = collision + wall * -r;
    }
}

vec3 Sphere::calc_position(float dt) const {
    return center + vel * dt + 0.5f * dt * dt * gravity;
}

vec3 Sphere::velocity_after_collision(float m1, const vec3& v1, float m2, const vec3& v2) {
    return (m1 * v1 + m2 * (2.f * v2 - v1)) / (m1 + m2);
}

bool Sphere::collide(Sphere& x, Sphere& y, float dt) {

    vec3 xc = x.calc_position(dt);
    vec3 yc = y.calc_position(dt);

    float r = x.r + y.r;
    
    if (vec3::distSq(xc, yc) < r*r) {

        // HACK: if we collided in the last frame and collision wasnt resolved correctly
        // do it now, with a crude approximation (well no approx. at all...)
        if (vec3::distSq(x.center, y.center) < r*r) {

            vec3 n1 = vec3::normalize(y.center - x.center);

            x.vel = x.vel.mag() * -n1;
            y.vel = y.vel.mag() * n1;

        } else {
            vec3 n1 = vec3::normalize(yc - xc);
            vec3 n2 = -n1;
        
            vec3 vx_coll = n1 * vec3::dot(x.vel, n1);
            vec3 vx_rest = x.vel - vx_coll;

            vec3 vy_coll = n2 * vec3::dot(y.vel, n2);
            vec3 vy_rest = y.vel - vy_coll;

            vec3 ux = (rand1() * 0.1f + 0.9f) * velocity_after_collision(x.mass, vx_coll, y.mass, vy_coll);
            vec3 uy = (rand1() * 0.1f + 0.9f) * velocity_after_collision(y.mass, vy_coll, x.mass, vx_coll);

            x.vel = ux + vx_rest;
            y.vel = uy + vy_rest;
        }

        return true;
    }

    return false;
}

void Camera::move_along_local_axis(const vec3& step) {
    frame.TranslateLocal(step.x, step.y, step.z);
//    origin += vec4::project3(orientation.getRotationMatrix().transpose() * vec4(step, 0.f));
}

void Camera::move_forward(float step) {
    move_along_local_axis(vec3(0, 0, 1) * step);
}

void Camera::move_right(float step) {
    move_along_local_axis(vec3(1, 0, 0) * step);
}

vec3 Camera::getOrigin() {
    M3DVector3f orig;
    frame.GetOrigin(orig);
    return toVec3(orig);
//    return origin;
}

vec3 Camera::getForwardVector() {
    M3DVector3f fw;
    frame.GetForwardVector(fw);
    return vec3::normalize(toVec3(fw));
//    return vec4::project3(orientation.getRotationMatrix().transpose() * vec4(0.f, 0.f, 1.f, 0.f));
}

void Camera::setOrigin(const vec3& orig) {
//    origin = orig;
    frame.SetOrigin(orig.x, orig.y, orig.z);
}

void Camera::facePoint(const vec3& pos) {
    vec3 fw = vec3::normalize(pos - getOrigin());
    frame.SetForwardVector(fw.x, fw.y, fw.z);
}

mat4 Camera::getCameraMatrix() {

    // GLFrame frame;

    // frame.SetOrigin(origin.x, origin.y, origin.z);
    
    // mat4 rot = orientation.getRotationMatrix().transpose();
    
    // vec3 fw  = vec4::project3(rot * vec4(0.f, 0.f, 1.f, 1.f));
    // vec3 up  = vec4::project3(rot * vec4(0.f, 1.f, 0.f, 1.f));
    
    // frame.SetForwardVector(fw.x, fw.y, fw.z);
    // frame.SetUpVector(up.x, up.y, up.z);

    M3DMatrix44f trans;
    frame.GetCameraMatrix(trans);
    return toMat4(trans);
}

void Camera::rotate(float rotx, float roty) {
    frame.RotateLocal(roty, 1.f, 0.f, 0.f);
    frame.RotateWorld(rotx, 0.f, 1.f, 0.f);
}

bool Cuboid::touchesWall(const Sphere& s, vec3& out_normal, vec3& out_collision) const {

    static const vec3 ground(0, -1.f, 0);

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

namespace {

void makeUnitCube(gltools::GenBatch<CubeVertex>& cube) {
    // cube.Begin(GL_QUADS, 24);

    CubeVertex v;
    
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

void addTriangle(gltools::GenBatch<SphereVertex>& s, const M3DVector3f vertices[3], const M3DVector3f normals[3], const M3DVector2f texCoords[3]) {

    for (uint32 i = 0; i < 3; ++i) {
        SphereVertex v;
        v.position = vec4(toVec3(vertices[i]), 1.f);
        v.normal = toVec3(normals[i]);
        s.add(v);
    }
}

// from the GLTools library (OpenGL Superbible)
void gltMakeSphere(gltools::GenBatch<SphereVertex>& sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks)
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

void makeSphere(gltools::GenBatch<SphereVertex>& sphere, float rad, int32 stacks, int32 slices) {
    gltMakeSphere(sphere, rad, stacks, slices);
}

} // namespace anon
