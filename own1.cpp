
#include <iostream>

#include <GL/glew.h>
#define FREEGLUT_STATIC
#include <GL/glut.h>

#include "defs.h"

using namespace std;

namespace {

    bool init() {
        return true;
    }

    void windowSizeChanged(int32 w, int32 h) {
        glViewport(0, 0, w, h);
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

            cerr << "unknown OpenGL Error occurred: " << err << endl;
        }

        if (str != 0)
            cerr << "OpenGL Error occurred: " << str << endl;

    }

    void render() {

        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        for (GLenum err; (err = glGetError()) != GL_NO_ERROR; )
            printGLError(err);
        
        glutSwapBuffers();
    }
}

int main(int argc, char *argv[]) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(400, 300);
    glutCreateWindow(__FILE__);
    glutReshapeFunc(windowSizeChanged);
    glutDisplayFunc(render);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
        return 1;
    }

    if (!init()) {
        cerr << "initialization failed" << endl;
        return 1;
    }

    glutMainLoop();
    
    return 0;
}
