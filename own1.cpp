
#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "defs.h"
#include "ShaderProgram.hpp"
#include "VertexBuffer.hpp"
#include "math/mat4.hpp"

using namespace std;

namespace {

    ShaderProgram shader;
    
    GLuint uniform_vec4_color;
    GLuint uniform_mat4_rotation;
    GLuint attribute_vec4_position;

    mat4 rotation = mat4::identity();
    
    VertexBuffer triangle;

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

            cerr << "unknown OpenGL Error occurred: " << err << endl;
        }

        if (str != 0)
            cerr << "OpenGL Error occurred: " << str << endl;

    }

    bool printGLErrors() {
        bool was_error = false;
        for (GLenum err; (err = glGetError()) != GL_NO_ERROR; was_error = true)
            printGLError(err);
        return was_error;
    }

    bool init() {

        if (!shader.compileAndLink("hello.vert", "hello.frag"))
            return false;

        uniform_vec4_color = glGetUniformLocation(shader.program, "color");
        uniform_mat4_rotation = glGetUniformLocation(shader.program, "rotation");
        attribute_vec4_position = glGetAttribLocation(shader.program, "position");

        triangle.add(vec4(-0.5f, -0.5f, 0.f, 1.f));
        triangle.add(vec4(+0.5f, -0.5f, 0.f, 1.f));
        triangle.add(vec4( 0.0f, +0.5f, 0.f, 1.f));

        triangle.send();

        return !printGLErrors();
    }

    void cleanup() {
        cerr << "shutting down..." << endl;
    }

    void windowSizeChanged(int32 w, int32 h) {
        glViewport(0, 0, w, h);
    }

    void idle() {
        int32 ms = glutGet(GLUT_ELAPSED_TIME);
        float angle = ms * 0.000125f * 2 * Math::PI;
        float sin, cos;
        Math::sincos(angle, &sin, &cos);
        
        rotation = mat4(vec4(cos, sin, 0.f, 0.f),
                        vec4(-sin, cos, 0.f, 0.f),
                        vec4(0.f, 0.f, 1.f, 0.f),
                        vec4(0.f, 0.f, 0.f, 1.f));
        
        glutPostRedisplay();
    }

    void render() {

        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glUseProgram(shader.program);

        glUniform4f(uniform_vec4_color, 1.f, 0.f, 0.f, 1.f);
        glUniformMatrix4fv(uniform_mat4_rotation, 1, GL_FALSE, (float *) &rotation);

        glEnableVertexAttribArray(attribute_vec4_position);
        triangle.use_as(attribute_vec4_position);
        triangle.draw();
        glDisableVertexAttribArray(attribute_vec4_position);

        printGLErrors();
        
        glutSwapBuffers();
    }
}

int main(int argc, char *argv[]) {

    glutInit(&argc, argv);
    cerr << "initialized glut" << endl;
    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(400, 300);
    glutCreateWindow(__FILE__);
    glutReshapeFunc(windowSizeChanged);
    glutIdleFunc(idle);    
    glutDisplayFunc(render);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
        return 1;
    } else {
        cerr << "initialized GLEW" << endl;
    }

    if (!init()) {
        cerr << "initialization failed" << endl;
        return 1;
    } else {
        cerr << "initialization successful" << endl;
    }

    atexit(cleanup);

    glutMainLoop(); // never returns
    
    return 0;
}
