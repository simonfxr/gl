
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

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
#include "math/Math.hpp"
#include "math/EulerAngles.hpp"
#include "math/transform.hpp"

#include "GameLoop.hpp"

static const vec3 gravity(0.f, -9.81f, 0.f);

namespace {

    struct Sphere {
        vec3 vel;
        float r;
        vec3 center;
        float mass;

        void move(float dt);
        vec3 calc_position(float dt) const;

        static void collide(Sphere& x, Sphere& y, float dt);

        static vec3 velocity_after_collision(float m1, const vec3& v1, float m2, const vec3& v2);

    } __attribute__((aligned(16)));

    struct Camera {
        vec3 origin;
        EulerAngles orientation;

        mat4 getModelViewMatrix() const; 

        void move_forward(float step);
        void move_right(float step);
        
    } __attribute__ ((aligned(16)));
}

namespace {

    void set_matrix(M3DMatrix44f dst, const mat4& src) {
        for (uint32 i = 0; i < 16; ++i)
            dst[i] = src.flat[i];
    }

    void printGLError(GLenum err) {
        const char *str;

        switch (err) {
            
#define err_case(e) case e: str = #e; break
            
            err_case(GL_NO_ERROR);
            err_case(GL_INVALID_ENUM);
            err_case(GL_INVALID_VALUE);
            err_case(GL_INVALID_OPERATION);
            err_case(GL_STACK_OVERFLOW);
            err_case(GL_STACK_UNDERFLOW);
            err_case(GL_OUT_OF_MEMORY);
            err_case(GL_TABLE_TOO_LARGE);

#undef err_case

        default:
            str = 0;

            std::cerr << "unknown OpenGL Error occurred: " << err << std::endl;
        }

        if (str != 0)
            std::cerr << "OpenGL Error occurred: " << str << std::endl;

    }

    bool printGLErrors() {
        bool was_error = false;
        for (GLenum err; (err = glGetError()) != GL_NO_ERROR; was_error = true)
            printGLError(err);
        return was_error;
    }

}

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

    GLBatch                 floorBatch;
    GLTriangleBatch         sphereBatch;
    
    Camera                  camera;

    uint32 fps_count;
    uint32 current_fps;
    float  fps_render_next;
    float  fps_render_last;

    uint64 frame_id;

    bool was_mouse_move;
    uint32 mouse_x, mouse_y;

    float sphere_speed;
    float sphere_mass;
    float sphere_rad;

    float game_speed;

    bool use_interpolation;
    uint64 last_frame_rendern;

    std::vector<Sphere> spheres;    

    Game(sf::Clock& _clock, sf::RenderWindow& _win);

    float now();
    void handle_events();
    void tick();
    void render(float interpolation);

    void init();
    
private:

    void update_fps_text(uint32 current_fps);
    void window_size_changed(uint32 width, uint32 height);
    void key_pressed(sf::Key::Code key);
    void mouse_moved(uint32 x, uint32 y);
    void mouse_pressed(sf::Mouse::Button button);
    void after_mouse_moves(uint32 x, uint32 y);
    void spawn_sphere();
    void render_sphere(const Sphere& s, float dt, const M3DVector3f vLightEyePos);
    void render_hud();
};

Game::Game(sf::Clock& _clock, sf::RenderWindow& _win)
    : clock(_clock), window(_win), loop(100, 5, 0)
{}

void Game::init() {

    frame_id = 0;
    last_frame_rendern = 0;

    sphere_speed = 10.f;
    sphere_mass = 1.f;
    sphere_rad = 0.3f;

    game_speed = 1.f;
    use_interpolation = true;
    
    window.EnableKeyRepeat(true);
    window.ShowMouseCursor(false);

    // Initialze Shader Manager
    shaderManager.InitializeStockShaders();
	
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // This make a sphere
    gltMakeSphere(sphereBatch, 1.f, 26, 13);
    	
    floorBatch.Begin(GL_LINES, 324);
    for (GLfloat x = -20.0; x <= 20.0f; x+= 0.5) {
        floorBatch.Vertex3f(x, 0.f, 20.0f);
        floorBatch.Vertex3f(x, 0.f, -20.0f);
        
        floorBatch.Vertex3f(20.0f, 0.f, x);
        floorBatch.Vertex3f(-20.0f, 0.f, x);
    }
    floorBatch.End();

    camera.origin = vec3(-20.f, 10.f, -20.f);
    camera.orientation = EulerAngles(Math::degToRad(45.f), Math::degToRad(30.f), 0.f);
    
    // vec3 fw = vec3::normalize(vec3(1.f, 0.f, 1.f));
    // cameraFrame.SetForwardVector(fw.x, fw.y, fw.z);

    fps_count = 0;
    fps_render_next = fps_render_last = now();
    
    window_size_changed(window.GetWidth(), window.GetHeight());

    printGLErrors();
}

float Game::now() {
    return clock.GetElapsedTime();
}

void Game::handle_events() {
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
            mouse_moved(e.MouseMove.X, e.MouseMove.Y); break;
        case sf::Event::MouseButtonPressed:
            mouse_pressed(e.MouseButton.Button); break;
        }
    }

    if (was_mouse_move) {
        was_mouse_move = false;
        after_mouse_moves(mouse_x, mouse_y);
    }
}

void Game::tick() {
    
    ++frame_id;

    float dt  = game_speed / loop.ticks_per_second;

    for (unsigned i = 1; i < spheres.size(); ++i)
        for (unsigned j = 0; j < i; ++j)
            Sphere::collide(spheres[i], spheres[j], dt);

    for (uint32 i = 0; i < spheres.size(); ++i)
        spheres[i].move(dt);
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

    glViewport(0, 0, width, height);

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

void Game::mouse_moved(uint32 x, uint32 y) {

    uint32 mx = window.GetWidth() / 2;
    uint32 my = window.GetHeight() / 2;

    if (mx == x && my == y)
        return;

    mouse_x = x;
    mouse_y = y;

    was_mouse_move = true;
}

void Game::after_mouse_moves(uint32 x, uint32 y) {

    uint32 mx = window.GetWidth() / 2;
    uint32 my = window.GetHeight() / 2;

    int32 dx = mx - x;
    int32 dy = y - my;

    // cameraFrame.RotateLocalY(dx * 0.001f);
    // cameraFrame.RotateLocalX(dy * 0.001f);

    window.SetCursorPosition(mx, my);
}

void Game::mouse_pressed(sf::Mouse::Button button) {
    if (button == sf::Mouse::Left)
        spawn_sphere();
}

static vec3 toVec3(M3DVector3f v3) {
    return vec3(v3[0], v3[1], v3[2]);
}

void Game::spawn_sphere() {

    static const vec4 forward = vec4(0.f, 0.f, -1.f, 0.f);

    vec3 dir = vec4::project3(camera.orientation.getRotationMatrix().transpose() * forward);

    Sphere s;
    s.center = camera.origin;
    s.vel    = dir * sphere_speed;
    s.r      = sphere_rad;
    s.mass   = sphere_mass;

    spheres.push_back(s);
}

static const GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f};
static const GLfloat vSphereColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
static const M3DVector4f vLightPos = { -20.0f, 30.0f, -20.0f, 1.0f };

void Game::render(float interpolation) {

    if (!use_interpolation && last_frame_rendern == frame_id)
        return;

    if (!use_interpolation)
        interpolation = 0.f;

    last_frame_rendern = frame_id;
    
    window.SetActive();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelViewMatrix.PushMatrix();
    
    M3DMatrix44f mCamera;
    set_matrix(mCamera, camera.getModelViewMatrix());
    modelViewMatrix.PushMatrix(mCamera);

    // Transform the light position into eye coordinates

    M3DVector4f vLightEyePos;
    m3dTransformVector4(vLightEyePos, vLightPos, mCamera);
		
    // Render the ground
    shaderManager.UseStockShader(GLT_SHADER_FLAT,
                                 transformPipeline.GetModelViewProjectionMatrix(),
                                 vFloorColor);	
    floorBatch.Draw();

    float dt = interpolation * game_speed / loop.ticks_per_second;

    for (uint32 i = 0; i < spheres.size(); ++i)
        render_sphere(spheres[i], dt, vLightEyePos);

    modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();    

    ++fps_count;

    render_hud();

    window.Display();

    printGLErrors();
}

void Game::render_sphere(const Sphere& s, float dt, const M3DVector3f vLightEyePos) {
    modelViewMatrix.PushMatrix();
    vec3 pos = s.calc_position(dt);
    modelViewMatrix.Translate(pos.x, pos.y, pos.z);
    modelViewMatrix.Scale(s.r, s.r, s.r);
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,
                                 transformPipeline.GetModelViewMatrix(), 
                                 transformPipeline.GetProjectionMatrix(),
                                 vLightEyePos, vSphereColor);
    sphereBatch.Draw();
    modelViewMatrix.PopMatrix();
}

void Game::render_hud() {

    float now = loop.real_time();
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
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    sf::ContextSettings glContext(24, 8, 0, 3, 3);
    sf::RenderWindow window(sf::VideoMode(800, 600), "sfml-sim", sf::Style::Default, glContext);
    sf::Clock clock;

    Game game(clock, window);
    game.init();
    int32 exit_code = game.loop.run(game);

    window.Close();

    return exit_code;
}

void Sphere::move(float dt) {
    center = calc_position(dt);
    vel += gravity * dt;

    if (center.y < r) {
        const vec3 ground(0.f, -1.f, 0.f);
        vec3 vel_par = ground * vec3::dot(vel, ground);
        vel = (vel - 2 * vel_par) * Math::sqrt(0.3f);
        center.y = r;
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
        vec3 n1 = vec3::normalize(yc - xc);
        vec3 n2 = -n1;
        
        vec3 vx_coll = n1 * vec3::dot(x.vel, n1);
        vec3 vx_rest = x.vel - vx_coll;

        vec3 vy_coll = n2 * vec3::dot(y.vel, n2);
        vec3 vy_rest = y.vel - vy_coll;

        vec3 ux = velocity_after_collision(x.mass, vx_coll, y.mass, vy_coll);
        vec3 uy = velocity_after_collision(y.mass, vy_coll, x.mass, vx_coll);

        x.vel = ux + vx_rest;
        y.vel = uy + vy_rest;

    }
}

void Camera::move_forward(float step) {
    mat4 rot = orientation.getRotationMatrix().transpose();
    vec4 forward = rot * vec4(0.f, 0.f, -1.f, 0.f);
    origin += vec4::project3(forward) * step;        
}

void Camera::move_right(float step) {
    mat4 rot = orientation.getRotationMatrix().transpose();
    vec4 forward = rot * vec4(0.f, 0.f, -1.f, 0.f);
    origin += vec4::project3(forward) * step;    
}

mat4 Camera::getModelViewMatrix() const {
    mat4 rotation = orientation.getRotationMatrix();
    mat4 trans, itrans;
    Transform::translate(origin, trans, itrans);
    return trans * rotation * itrans;
}
