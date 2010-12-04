// SphereWorld.cpp
// OpenGL SuperBible
// New and improved (performance) sphere world
// Program by Richard S. Wright Jr.

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLFrame.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <math.h>
#include <stdio.h>
#include <iostream>
#include <vector>

#include "math/vec3.hpp"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLShaderManager		shaderManager;			// Shader Manager
GLMatrixStack		modelViewMatrix;		// Modelview Matrix
GLMatrixStack		projectionMatrix;		// Projection Matrix
GLFrustum			viewFrustum;			// View Frustum
GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline

GLBatch				floorBatch;
GLTriangleBatch     sphereBatch;
GLFrame             cameraFrame;

struct Sphere {
    vec3 vel;
    float r;
    vec3 center;
    float mass;
} __attribute__((aligned(16)));

std::vector<Sphere> spheres;

        
//////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering
// context. 
void SetupRC()
{
    // Initialze Shader Manager
    shaderManager.InitializeStockShaders();
	
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
    // This make a sphere
    gltMakeSphere(sphereBatch, 0.1f, 26, 13);
    	
    floorBatch.Begin(GL_LINES, 324);
    for(GLfloat x = -20.0; x <= 20.0f; x+= 0.5) {
        floorBatch.Vertex3f(x, -0.55f, 20.0f);
        floorBatch.Vertex3f(x, -0.55f, -20.0f);
        
        floorBatch.Vertex3f(20.0f, -0.55f, x);
        floorBatch.Vertex3f(-20.0f, -0.55f, x);
    }
    floorBatch.End();

    cameraFrame.SetOrigin(-20.f, 10.f, -20.f);
    vec3 fw = vec3(1.f, 0.f, 1.f).normal();
    cameraFrame.SetForwardVector(fw.x, fw.y, fw.z);
}

static int win_width = 1;
static int win_height = 1;
static bool recenter_cursor = true;

void MouseMoved(int, int);

static void mouse_callback_set(bool on) {
    if (on) {
        glutMotionFunc(MouseMoved);
        glutPassiveMotionFunc(MouseMoved);
    } else {
        glutMotionFunc(NULL);
        glutPassiveMotionFunc(NULL);
    }
}

///////////////////////////////////////////////////
// Screen changes size or is initialized
void ChangeSize(int nWidth, int nHeight)
{
    win_height = nHeight;
    win_width = nWidth;
    glViewport(0, 0, nWidth, nHeight);

    mouse_callback_set(false);
    glutWarpPointer(nWidth / 2, nHeight / 2);
    mouse_callback_set(true);
	
    // Create the projection matrix, and load it on the projection matrix stack
    viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    // Set the transformation pipeline to use the two matrix stacks 
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

static const unsigned ANIM_TICKS_PER_SECOND = 50;
static const float ANIM_TICK_LEN = 1.0 / ANIM_TICKS_PER_SECOND;

static float next_tick = 0;

static void animate(float dt, float acc);

static float sphere_speed0 = 3.f;

static float animation_speed = 0.03f;

static const vec3 g(0.f, -9.81f, 0.f);

static Sphere make_sphere(const vec3& center, const vec3& dir) {
    Sphere s;
    s.center = center;
    s.vel    = dir.normal() * sphere_speed0;
    s.mass   = 1.f;
    s.r      = 1.f;

    return s;
}

static void anim_tick(float now) {
    if (now > next_tick) {
        next_tick = now + ANIM_TICK_LEN;

        animate(ANIM_TICK_LEN, animation_speed);
    }
}

static std::ostream& operator << (std::ostream& out, const vec3& v) {
    return out << "(" << v.x << ";" << v.y << ";" << v.z << ")";
}

static const vec3 ground(0.f, -1.f, 0.f);

static void resolve_collision(Sphere& x, Sphere& y) {

    vec3 xc = x.center + x.vel;
    vec3 yc = y.center + y.vel;

    float r = x.r + y.r;
    
    if (vec3::distSq(xc, yc) < r*r) {
        vec3 n1 = vec3::normal(xc - yc);
        vec3 n2 = -n1;
        
        vec3 vx_coll = n1 * vec3::dot(x.vel, n1);
        vec3 vx_rest = x.vel - vx_coll;
    }
}

static void __attribute__((noinline)) animate(const float dt, float acc) {
    for (unsigned i = 0; i < spheres.size(); ++i) {
        Sphere &s = spheres[i];

        s.vel    += g * (dt * acc);
        s.center += s.vel * acc;

        if (s.center.y - s.r < 0) {
            const vec3 ground(0.f, -1.f, 0.f);
            vec3 vel_par = ground * vec3::dot(s.vel, ground);
            s.vel = (s.vel - 2 * vel_par) * Math::sqrt(0.2f);
            s.center.y = s.r;
        }
    }
}
        
// Called to draw scene
void RenderScene(void)
{
    // Color values
    static GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f};
//    static GLfloat vTorusColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    static GLfloat vSphereColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };

    // Time Based animation
    static CStopWatch	rotTimer;
    float now = rotTimer.GetElapsedSeconds();

    anim_tick(now);
	
    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    
    // Save the current modelview matrix (the identity matrix)
    modelViewMatrix.PushMatrix();
    
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.PushMatrix(mCamera);

    // Transform the light position into eye coordinates
    M3DVector4f vLightPos = { -20.0f, 30.0f, -20.0f, 1.0f };
    M3DVector4f vLightEyePos;
    m3dTransformVector4(vLightEyePos, vLightPos, mCamera);
		
    // Draw the ground
    shaderManager.UseStockShader(GLT_SHADER_FLAT,
                                 transformPipeline.GetModelViewProjectionMatrix(),
                                 vFloorColor);	
    floorBatch.Draw();

    for (unsigned i = 0; i < spheres.size(); ++i) {
        modelViewMatrix.PushMatrix();
        const Sphere& s = spheres[i];
        modelViewMatrix.Translate(s.center.x, s.center.y, s.center.z);
        modelViewMatrix.Scale(s.r, s.r, s.r);
        shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,
                                     transformPipeline.GetModelViewMatrix(), 
                                     transformPipeline.GetProjectionMatrix(),
                                     vLightEyePos, vSphereColor);
        sphereBatch.Draw();
        
        modelViewMatrix.PopMatrix();
    }
    
    // for(int i = 0; i < NUM_SPHERES; i++) {
    //     modelViewMatrix.PushMatrix();
    //     modelViewMatrix.MultMatrix(spheres[i]);
    //     shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), 
    //                                  transformPipeline.GetProjectionMatrix(), vLightEyePos, vSphereColor);
    //     sphereBatch.Draw();
    //     modelViewMatrix.PopMatrix();
    // }

    // // Draw the spinning Torus
    // modelViewMatrix.Translate(0.0f, 0.0f, -2.5f);
    
    // // Save the Translation
    // modelViewMatrix.PushMatrix();
    
    // // Apply a rotation and draw the torus
    // modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
    // shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), 
    //                              transformPipeline.GetProjectionMatrix(), vLightEyePos, vTorusColor);
    // torusBatch.Draw();
    // modelViewMatrix.PopMatrix(); // "Erase" the Rotation from before

    // Apply another rotation, followed by a translation, then draw the sphere
    // modelViewMatrix.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
    // modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);
    // shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), 
    //                              transformPipeline.GetProjectionMatrix(), vLightEyePos, vSphereColor);
    // sphereBatch.Draw();

    // Restore the previous modleview matrix (the identity matrix)
    modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();    
    // Do the buffer Swap
    glutSwapBuffers();

    if (recenter_cursor) {
        mouse_callback_set(false);
        glutWarpPointer(win_width / 2, win_height / 2);
        mouse_callback_set(true);        
        recenter_cursor = false;
    }
        
    // Tell GLUT to do it again
    glutPostRedisplay();
}

// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
{
    UNUSED(x); UNUSED(y);
    float linear = 0.1f;
//    float angular = float(m3dDegToRad(5.0f));
	
    if(key == GLUT_KEY_UP)
        cameraFrame.MoveForward(linear);
	
    if(key == GLUT_KEY_DOWN)
        cameraFrame.MoveForward(-linear);
	
    if(key == GLUT_KEY_LEFT)
        cameraFrame.MoveRight(linear);
	
    if(key == GLUT_KEY_RIGHT)
        cameraFrame.MoveRight(-linear);
//        cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);		
}

void MouseMoved(int x, int y) {

//    std::cerr << "mouse moved: " << x << ";" << y << std::endl;

    float dx = x - float(win_width) / 2;
    float dy = y - float(win_height) / 2;

    cameraFrame.RotateLocalY(-dx * 0.0015f);
    cameraFrame.RotateLocalX(+dy * 0.0015f);

//    cameraFrame.RotateLocal(0.05f, dy, dx, 0);

    
    recenter_cursor = true;
}

static vec3 toVec3(M3DVector3f v3) {
    return vec3(v3[0], v3[1], v3[2]);
}

void MouseEvent(int button, int state, int x, int y) {

    UNUSED(x); UNUSED(y);
    
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        M3DVector3f orig;
        cameraFrame.GetOrigin(orig);
        M3DVector3f dir;
        cameraFrame.GetForwardVector(dir);
        Sphere s = make_sphere(toVec3(orig), toVec3(dir));
        spheres.push_back(s);
        std::cerr << "made sphere: " << s.center.x << ";" << s.center.y << ";" << s.center.z << std::endl;
        std::cerr << "  speed: " << s.vel.x << ";" << s.vel.y << ";" << s.vel.z << std::endl;
    }
}

int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
  
    glutCreateWindow("sim");

    glutSetCursor(GLUT_CURSOR_NONE);
 
    glutSpecialFunc(SpecialKeys);
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutMouseFunc(MouseEvent);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    SetupRC();
    glutMainLoop();    
    return 0;
}
