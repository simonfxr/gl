#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <limits.h>

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLFrame.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "math/vec3.hpp"
#include "math/ivec3.hpp"
#include "math/Math.hpp"
#include "math/EulerAngles.hpp"
#include "math/transform.hpp"

#include "GameLoop.hpp"
#include "gltools.hpp"

static const vec3 gravity(0.f, -9.81f, 0.f);

static void makeUnitCube(GLBatch& cube);

namespace {

struct Sphere;

struct Cuboid {
    vec3 corner_min;
    vec3 corner_max;
    bool touchesWall(const Sphere& s, vec3& out_normal, vec3& out_collision) const;
};

struct Sphere {
    vec3 vel;
    float r;
    vec3 center;
    float mass;
    vec4 color;

    void move(const Cuboid& room, float dt);
    vec3 calc_position(float dt) const;

    static void collide(Sphere& x, Sphere& y, float dt);

    static vec3 velocity_after_collision(float m1, const vec3& v1, float m2, const vec3& v2);

} __attribute__((aligned(16)));

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

}

namespace {

void set_matrix(M3DMatrix44f dst, const mat4& src) {
    for (uint32 i = 0; i < 16; ++i)
        dst[i] = src.flat[i];
}

std::ostream& operator << (std::ostream& out, const vec3& v) {
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

} // namespace anon

static const float FPS_RENDER_CYCLE = 1.f;

struct Game : public GameLoop::Game {
public:
    
    sf::Clock &clock;
    sf::RenderWindow &window;
    GameLoop loop;
    sf::Text txtFps;

    GLShaderManager         shaderManager;              // Shader Manager
    GLMatrixStack           modelViewMatrix;            // Modelview Matrix
    GLMatrixStack           projectionMatrix;           // Projection Matrix
    GLFrustum               viewFrustum;                // View Frustum
    GLGeometryTransform     transformPipeline;          // Geometry Transform Pipeline

    // GLBatch                 cubeBatch;
    GLBatch                 floorBatch;
    GLTriangleBatch         sphereBatch;

    GLBatch wallBatch;
    
    Camera                  camera;
    Cuboid                  room;

    uint32 fps_count;
    uint32 current_fps;
    float  fps_render_next;
    float  fps_render_last;

    uint64 frame_id;

    uint32 mouse_dx, mouse_dy;

    float sphere_speed;
    float sphere_mass;
    float sphere_rad;

    float game_speed;

    bool use_interpolation;
    uint64 last_frame_rendered;

    std::vector<Sphere> spheres;    

    Game(sf::Clock& _clock, sf::RenderWindow& _win);

    float now();
    void handleEvents();
    void tick();
    void render(float interpolation);

    void init();
    
private:

    void update_fps_text(uint32 current_fps);
    void window_size_changed(uint32 width, uint32 height);
    void key_pressed(sf::Key::Code key);
    void mouse_moved(int32 dx, int32 dy);
    void mouse_pressed(sf::Mouse::Button button);
    void spawn_sphere();
    void render_sphere(const Sphere& s, float dt, const M3DVector3f vLightEyePos);
    void render_hud();
    void render_walls(const M3DVector3f vLightEyePos);
    void resolve_collisions(float dt);
};

Game::Game(sf::Clock& _clock, sf::RenderWindow& _win)
    : clock(_clock), window(_win), loop(100, 5, 0)
{}

void Game::init() {

    frame_id = 0;
    last_frame_rendered = 0;

    sphere_speed = 10.f;
    sphere_mass = 1.f;
    sphere_rad = 0.3f;

    game_speed = 1.f;
    use_interpolation = true;
    
    window.EnableKeyRepeat(true);
    window.ShowMouseCursor(false);

    room.corner_min = vec3(-20.f, 0, -20.f);
    room.corner_max = vec3(+20.f, 20.f, 20.f);

    makeUnitCube(wallBatch);

    window.SetActive();

    // Initialze Shader Manager
    shaderManager.InitializeStockShaders();
	
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

    // This make a sphere
    gltMakeSphere(sphereBatch, 1.f, 26, 13);

    const vec3 lln = room.corner_min;
    const vec3 dim = room.corner_max - room.corner_min;
    	
    floorBatch.Begin(GL_LINES, 324);
    for (GLfloat x = -20.0; x <= 20.0f; x+= 0.5) {
        floorBatch.Vertex3f(x, 0.f, 20.0f);
        floorBatch.Vertex3f(x, 0.f, -20.0f);
        
        floorBatch.Vertex3f(20.0f, 0.f, x);
        floorBatch.Vertex3f(-20.0f, 0.f, x);
    }
    floorBatch.End();

    // camera.origin = vec3(-20.f, 10.f, -20.f);
    // camera.orientation = EulerAngles(0, Math::PI, 0);
    // camera.orientation.canonize();

    // camera.setOrigin(room.corner_min);
    camera.setOrigin(vec3(-19.f, 10.f, -19.f));
    camera.facePoint(vec3(0.f, 10.f, 0.f));
    
    fps_count = 0;
    fps_render_next = fps_render_last = 0.f;
    
    window_size_changed(window.GetWidth(), window.GetHeight());
}

float Game::now() {
    return clock.GetElapsedTime();
}

void Game::handleEvents() {

    uint32 win_w = window.GetWidth();
    uint32 win_h = window.GetHeight();

    uint32 mouse_x = win_w / 2;
    uint32 mouse_y = win_h / 2;
    
    sf::Event e;
    while (window.GetEvent(e)) {
        switch (e.Type) {
        case sf::Event::Closed:
            loop.exit(); break;
        case sf::Event::Resized:
            window_size_changed(e.Size.Width, e.Size.Height); break;
        case sf::Event::KeyPressed:
            key_pressed(e.Key.Code); break;
        case sf::Event::MouseMoved:
            mouse_x = e.MouseMove.X;
            mouse_y = e.MouseMove.Y;
            break;
        case sf::Event::MouseButtonPressed:
            mouse_pressed(e.MouseButton.Button); break;
        }
    }

    int32 dx = int32(win_w / 2) - mouse_x;
    int32 dy = mouse_y - int32(win_h / 2);

    if (dx != 0 || dy != 0) {
        window.SetCursorPosition(win_w / 2, win_h / 2);
        mouse_moved(dx, dy);
    }
}

void Game::tick() {
    
    ++frame_id;

    float dt  = game_speed / loop.ticks_per_second;

    resolve_collisions(dt);

    for (uint32 i = 0; i < spheres.size(); ++i)
        spheres[i].move(room, dt);

    Sphere cam;
    cam.center = camera.getOrigin();
    cam.r = 0.01f;

    vec3 norm;
    vec3 col;
    if (room.touchesWall(cam, norm, col)) {
        camera.setOrigin(col);
    }
}

static ivec3 calcTile(const vec3& isize, const vec3& p) {
    return ivec3(vec3::compMult(isize, p)); 
}

struct Tile {
    uint32 object;
    Tile *nxt;
};

static void insertTile(Tile* &t, uint32 id, std::vector<Tile *> allocs) {
    Tile *nt = new Tile();
    allocs.push_back(nt);
    nt->object = id;
    nt->nxt = t;
    t = nt;
}

static void collTile(std::vector<Sphere> ss, Sphere& s, uint32 i, const Tile *t, float dt) {
    while (t != 0) {
        if (t->object < i)
            Sphere::collide(s, ss[t->object], dt);
        t = t->nxt;
    }
}

void Game::resolve_collisions(float dt) {

    float t0 = now();

    const uint32 dim = 50;
    const vec3 diff = room.corner_max - room.corner_min;
    const vec3 isize = vec3(dim / diff.x, dim / diff.y, dim / diff.z);
    const vec3 low  = room.corner_min;

    const ivec3 tmin = ivec3(0);
    const ivec3 tmax = ivec3(dim - 1);

    Tile *tiles[dim][dim][dim];
    std::vector<Tile *> allocs;

    memset(tiles, 0, dim * dim * dim * sizeof tiles[0][0][0]);

    for (uint32 i = 0; i < spheres.size(); ++i) {
        
        Sphere& s = spheres[i];
        vec3 p = s.calc_position(dt) - low;

        ivec3 tileL = ivec3::max(tmin, calcTile(isize, p - vec3(s.r)));
        ivec3 tileH = ivec3::min(tmax, calcTile(isize, p + vec3(s.r)));

        for (int32 x = tileL.x; x <= tileH.x; ++x)
            for (int32 y = tileL.y; y <= tileH.y; ++y)
                for (int32 z = tileL.z; z <= tileH.z; ++z)
                    insertTile(tiles[x][y][z], i, allocs);
    }

    for (unsigned i = 0; i < spheres.size(); ++i) {
        
        Sphere& s = spheres[i];
        vec3 p = s.calc_position(dt) - low;

        ivec3 tileL = ivec3::max(tmin, calcTile(isize, p - vec3(s.r)));
        ivec3 tileH = ivec3::min(tmax, calcTile(isize, p + vec3(s.r)));

        for (int32 x = tileL.x; x <= tileH.x; ++x)
            for (int32 y = tileL.y; y <= tileH.y; ++y)
                for (int32 z = tileL.z; z <= tileH.z; ++z)
                    collTile(spheres, s, i, tiles[x][y][z], dt);
    }

    for (uint32 i = 0; i < allocs.size(); ++i)
        delete allocs[i];

    std::cerr << "resolve_collisions: " << (now() - t0) << " seconds" << std::endl;
}

template <typename T>
static std::string to_string(T x) {
    std::stringstream out;
    out << x;
    return out.str();
}

void Game::update_fps_text(uint32 current_fps) {
    txtFps = sf::Text(to_string(current_fps));
    txtFps.SetCharacterSize(16);
    txtFps.SetPosition(2, 0);
    txtFps.SetColor(sf::Color(255, 255, 0, 180));
}

void Game::window_size_changed(uint32 width, uint32 height) {
    update_fps_text(current_fps);

    GL_CHECK(glViewport(0, 0, width, height));

    // Create the projection matrix, and load it on the projection matrix stack
    viewFrustum.SetPerspective(35.0f, float(width) / float(height), 1.0f, 100.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    // Set the transformation pipeline to use the two matrix stacks 
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

void Game::key_pressed(sf::Key::Code key) {
    float linear = 0.15f;

    using namespace sf::Key;

    switch (key) {
    case W: camera.move_forward(linear); break;
    case S: camera.move_forward(-linear); break;
    case A: camera.move_right(linear); break;
    case D: camera.move_right(-linear); break;
    case M: sphere_mass += 0.1f; break;
    case N: sphere_mass -= 0.1f; break;
    case R: sphere_rad += 0.1f; break;
    case E: sphere_rad -= 0.1f; break;
    case P: sphere_speed += 0.5f; break;
    case O: sphere_speed -= 0.5f; break;
    case U: game_speed += 0.05f; break;
    case Z: game_speed -= 0.05f; break;
    case I: use_interpolation = !use_interpolation; break;
    case B: loop.pause(!loop.paused()); break;
    }

    sphere_mass = std::max(0.1f, sphere_mass);
    sphere_rad = std::max(0.1f, sphere_rad);
    sphere_speed = std::max(0.5f, sphere_speed);
    game_speed = std::max(0.f, game_speed);
}

void Game::mouse_moved(int32 dx, int32 dy) {

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

void Game::mouse_pressed(sf::Mouse::Button button) {
    if (button == sf::Mouse::Left)
        spawn_sphere();
}

static const GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f};
static const vec4 SPHERE_COLOR = vec4(0.f, 0.f, 1.f, 1.f);
static const M3DVector4f vLightPos = { -20.0f, 30.0f, -20.0f, 1.0f };
static const M3DVector4f vWallColor = { 0.6f, 0.6f, 0.6f, 1.f };

static float rand1() {
    return rand() * (1.f / RAND_MAX);
}

static vec4 randomColor() {
    return vec4(rand1(), rand1(), rand1(), 1.f);
}

void Game::spawn_sphere() {

    Sphere s;
    s.center = camera.getOrigin();
    s.vel    = camera.getForwardVector() * sphere_speed;
    s.r      = sphere_rad;
    s.mass   = sphere_mass;
    s.color  = randomColor();

    spheres.push_back(s);
}

void Game::render(float interpolation) {

    if (!use_interpolation && last_frame_rendered == frame_id)
        return;

    if (!use_interpolation)
        interpolation = 0.f;

    last_frame_rendered = frame_id;
    
    window.SetActive();

    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    modelViewMatrix.PushMatrix();

    M3DMatrix44f mCamera;
    set_matrix(mCamera, camera.getCameraMatrix());
    modelViewMatrix.PushMatrix(mCamera);

    // Transform the light position into eye coordinates

    M3DVector4f vLightEyePos;
    m3dTransformVector4(vLightEyePos, vLightPos, mCamera);

    // modelViewMatrix.Translate(room.corner_min.x, room.corner_min.y, room.corner_min.z);
    // const vec3 dim = room.corner_max - room.corner_min;
    // modelViewMatrix.Scale(dim.x, dim.y, dim.z);
		
    // Render the ground
    GL_CHECK(shaderManager.UseStockShader(GLT_SHADER_FLAT,
                                          transformPipeline.GetModelViewProjectionMatrix(),
                                          vFloorColor));
    
    GL_CHECK(floorBatch.Draw());

    render_walls(vLightEyePos);

    float dt = interpolation * game_speed / loop.ticks_per_second;

    for (uint32 i = 0; i < spheres.size(); ++i)
        render_sphere(spheres[i], dt, vLightEyePos);

    modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();    

    ++fps_count;

    render_hud();

    window.Display();

    gltools::printErrors(std::cerr);
}

void Game::render_sphere(const Sphere& s, float dt, const M3DVector3f vLightEyePos) {

    M3DVector4f color = { s.color.x, s.color.y, s.color.z, s.color.w };
    
    modelViewMatrix.PushMatrix();
    vec3 pos = s.calc_position(dt);
    modelViewMatrix.Translate(pos.x, pos.y, pos.z);
    modelViewMatrix.Scale(s.r, s.r, s.r);
    GL_CHECK(shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,
                                          transformPipeline.GetModelViewMatrix(), 
                                          transformPipeline.GetProjectionMatrix(),
                                          vLightEyePos, color));
    sphereBatch.Draw();
    modelViewMatrix.PopMatrix();
}

void Game::render_walls(const M3DVector3f vLightEyePos) {

    modelViewMatrix.PushMatrix();

    vec3 center = room.corner_max;
    center.y = room.corner_min.y;
    center = 0.5 * (center + room.corner_min);

    vec3 diff = 0.5 * (room.corner_max - room.corner_min);

    modelViewMatrix.Translate(center.x, center.y, center.z);
    modelViewMatrix.Scale(diff.x, diff.y * 2 , diff.z);
    
    GL_CHECK(shaderManager.UseStockShader(GLT_SHADER_FLAT,
                                          transformPipeline.GetModelViewProjectionMatrix(),
                                          vWallColor));	


    // GL_CHECK(shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,
    //                                       transformPipeline.GetModelViewMatrix(), 
    //                                       transformPipeline.GetProjectionMatrix(),
    //                                       vLightEyePos, vWallColor));

    wallBatch.Draw();

    modelViewMatrix.PopMatrix();    
}

void Game::render_hud() {

    float now = loop.realTime();
    if (now >= fps_render_next) {

        current_fps = fps_count / (now - fps_render_last);
        fps_render_next = now + FPS_RENDER_CYCLE;
        fps_render_last = now;
        fps_count = 0;

        update_fps_text(current_fps);
    }

    sf::Color c(0, 255, 255, 180);

    uint32 height = txtFps.GetRect().Height + 2;
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

    window.SaveGLStates();
    
    window.Draw(txtFps);
    window.Draw(txtSpeed);
    window.Draw(txtMass);
    window.Draw(txtRad);
    window.Draw(txtAnimSpeed);
    window.Draw(txtInter);
    
    window.RestoreGLStates();
}

int main(int argc, char *argv[]) {

    UNUSED(argc);
    
    if (argv[0] != 0)
        gltSetWorkingDirectory(argv[0]);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    sf::ContextSettings glContext;
    glContext.MajorVersion = 3;
    glContext.MinorVersion = 3;
    glContext.DebugContext = true;
    sf::RenderWindow window(sf::VideoMode(800, 600), "sfml-sim", sf::Style::Default, glContext);

    window.SetActive();
    GL_CHECK((void)0);
    
    sf::Clock clock;

    Game game(clock, window);
    
    game.init();

    return game.loop.run(game);
}

void Sphere::move(const Cuboid& room, float dt) {
    center = calc_position(dt);
    vel += gravity * dt;

    vec3 wall;
    vec3 collision;
    if (room.touchesWall(*this, wall, collision)) {
//        const vec3 ground(0.f, -1.f, 0.f);
        vec3 vel_par = (rand1() * 0.1f + 0.9f) * (wall * vec3::dot(vel, wall));
        vel = (vel - 2 * vel_par) * Math::sqrt(0.6f);
        center = collision + wall * -r;
    }
}

vec3 Sphere::calc_position(float dt) const {
    return center + vel * dt + 0.5f * dt * dt * gravity;
}

vec3 Sphere::velocity_after_collision(float m1, const vec3& v1, float m2, const vec3& v2) {
    return (m1 * v1 + m2 * (2.f * v2 - v1)) / (m1 + m2);
}

void Sphere::collide(Sphere& x, Sphere& y, float dt) {

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
    }
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

static void makeUnitCube(GLBatch& cube) {

    cube.Begin(GL_QUADS, 24);

    cube.Vertex3f(-1.0f, -1.0f,  1.0f);
    cube.Vertex3f( 1.0f, -1.0f,  1.0f);
    cube.Vertex3f( 1.0f,  1.0f,  1.0f);
    cube.Vertex3f(-1.0f,  1.0f,  1.0f);

    cube.Vertex3f(-1.0f, -1.0f, -1.0f);
    cube.Vertex3f(-1.0f,  1.0f, -1.0f);
    cube.Vertex3f( 1.0f,  1.0f, -1.0f);
    cube.Vertex3f( 1.0f, -1.0f, -1.0f);

    cube.Vertex3f(-1.0f,  1.0f, -1.0f);
    cube.Vertex3f(-1.0f,  1.0f,  1.0f);
    cube.Vertex3f( 1.0f,  1.0f,  1.0f);
    cube.Vertex3f( 1.0f,  1.0f, -1.0f);

    cube.Vertex3f(-1.0f, -1.0f, -1.0f);
    cube.Vertex3f( 1.0f, -1.0f, -1.0f);
    cube.Vertex3f( 1.0f, -1.0f,  1.0f);
    cube.Vertex3f(-1.0f, -1.0f,  1.0f);

    cube.Vertex3f( 1.0f, -1.0f, -1.0f);
    cube.Vertex3f( 1.0f,  1.0f, -1.0f);
    cube.Vertex3f( 1.0f,  1.0f,  1.0f);
    cube.Vertex3f( 1.0f, -1.0f,  1.0f);

    cube.Vertex3f(-1.0f, -1.0f, -1.0f);
    cube.Vertex3f(-1.0f, -1.0f,  1.0f);
    cube.Vertex3f(-1.0f,  1.0f,  1.0f);
    cube.Vertex3f(-1.0f,  1.0f, -1.0f);

    cube.End();
}

