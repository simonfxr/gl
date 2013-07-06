
#include "defs.hpp"

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/real.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

#include "glt/Transformations.hpp"

#include <iostream>
#include <cstdlib>

using namespace math;

struct State {
    GLuint program;
    
    GLuint vertex_array;
    GLuint vertex_buffer;

    GLint uniform_mvp;
    GLint uniform_mv;
    GLint attrib_position;
    GLint attrib_color;
    
};

struct Vertex {
    vec3_t position;
    vec3_t color;
};


#define X .525731112119133606 
#define Z .850650808352039932

const GLfloat VERTEX_DATA[12][3] = {    
    {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},    
    {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},    
    {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0} 
};

const GLushort ELEMENT_DATA[20][3] = { 
    {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},    
    {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},    
    {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
    {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
};

void glfw_error_callback(int, const char* description) {
    std::cerr << description << std::endl;
}

void APIENTRY opengl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, void *userParam) {
    UNUSED(source); UNUSED(type); UNUSED(id);
    UNUSED(severity); UNUSED(length); UNUSED(userParam);
    std::cerr << message << std::endl;
}

#define DEF_GL_SHADER(name, src) const char * const name = "#version 330\n" #src

DEF_GL_SHADER(VERTEX_SHADER,

              uniform mat4 mvpMatrix;
              uniform mat4 mvMatrix;

              in vec3 position;
              in vec3 color;

              out vec3 fragColor;
              
              void main() {
                  gl_Position = mvpMatrix * vec4(position, 1);
                  fragColor = color;
              });

DEF_GL_SHADER(FRAGMENT_SHADER,

              in vec3 fragColor;
              
              out vec4 color;

              void main() {
                  color.rgb = fragColor;
                  color.a = 1;
              });

bool initProgram(State& s) {
    GLuint& program = s.program;
    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar *vert_source = (const GLchar *) VERTEX_SHADER;
    glShaderSource(vert_shader, 1, &vert_source, NULL);
    glCompileShader(vert_shader);
    
    const GLchar *frag_source = (const GLchar *) FRAGMENT_SHADER;
    glShaderSource(frag_shader, 1, &frag_source, NULL);
    glCompileShader(frag_shader);

    program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    glLinkProgram(program);
    if (glGetError() != GL_NO_ERROR)
        return false;

    s.uniform_mvp = glGetUniformLocation(s.program, "mvpMatrix");
    s.uniform_mv = glGetUniformLocation(s.program, "mvMatrix");
    
    s.attrib_position = glGetAttribLocation(s.program, "position");
    s.attrib_color = glGetAttribLocation(s.program, "color");

    if (glGetError() != GL_NO_ERROR)
        return false;

    return true;
}

bool initVertexArray(State& s) {


    std::vector<Vertex> vertices;

    for (int i = 0; i < 20; ++i) {
        Vertex v;
        v.color = vec3(real(i / 5) * (1.f / 4), real(i % 5) * (1.f / 4), 0);

        for (int k = 0; k < 3; ++k) {
            v.position = vec3(VERTEX_DATA[ELEMENT_DATA[i][k]]);
            vertices.push_back(v);
        }
    }

    glGenBuffers(1, &s.vertex_buffer);
    
    glBindBuffer(GL_ARRAY_BUFFER, s.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof (Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (glGetError() != GL_NO_ERROR)
        return false;

    glGenVertexArrays(1, &s.vertex_array);

    glBindVertexArray(s.vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, s.vertex_buffer);
    glVertexAttribPointer(s.attrib_position, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid *) offsetof(Vertex, position));
    glVertexAttribPointer(s.attrib_color, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid *) offsetof(Vertex, color));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(s.attrib_position);
    glEnableVertexAttribArray(s.attrib_color);
    glBindVertexArray(0);
    
    if (glGetError() != GL_NO_ERROR)
        return false;
    
    return true;
}

bool init(State& s) {
    return initProgram(s) && initVertexArray(s);
}

int main() {
    GLFWwindow* window = 0;
    int ret = EXIT_FAILURE;
    GLenum glew_err = GLEW_OK;
    GLenum gl_err = GL_NO_ERROR;
    State state;

    if (!glfwInit())
        goto err0;

    glfwSetErrorCallback(glfw_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    window = glfwCreateWindow(640, 480, "opengl-raw", NULL, NULL);
    if (!window)
        goto err1;

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glew_err = glewInit();
    if (glew_err != GLEW_OK)
        goto err2;
    
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    glDebugMessageCallbackARB(opengl_error_callback, NULL);
    
    gl_err = glGetError();
    if (gl_err != GL_NO_ERROR)
        goto err2;

    if (!init(state))
        goto err2;

    glEnable(GL_DEPTH_TEST);

    for (;;) {
        glfwPollEvents();
        if (glfwWindowShouldClose(window))
            break;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClearBufferfv(GL_COLOR, 0, &vec4(1.f, 1.f, 1.f, 1.f)[0]);
        glClear(GL_DEPTH_BUFFER_BIT);

        real time = real(glfwGetTime());

        mat4_t rotationMatrix = mat4(glt::rotationMatrix(time * 0.2f, normalize(vec3(1.f, 1.f, 1.f))));
        mat4_t projectionMatrix = glt::perspectiveProjection(45.f, real(width) / real(height), 0.5f, 600.f);
        mat4_t mvMatrix = mat4();
        mvMatrix[3] = vec4(0.f, 0.f, -3.f, 1.f);
        mvMatrix = mvMatrix * rotationMatrix;

        mat4_t mvpMatrix = projectionMatrix * mvMatrix;

        glUseProgram(state.program);
        glUniformMatrix4fv(state.uniform_mvp, 1, GL_FALSE, &mvpMatrix(0, 0));
        glUniformMatrix4fv(state.uniform_mv, 1, GL_FALSE, &mvMatrix(0, 0));

        glBindVertexArray(state.vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 20 * 3);

        glfwSwapBuffers(window);
    }
    
    ret = EXIT_SUCCESS;

err2:
    glfwDestroyWindow(window);
err1:
    glfwTerminate();
err0:
    
    return ret;
}
