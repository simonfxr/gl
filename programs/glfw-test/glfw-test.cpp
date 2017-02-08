#define GLEW_NO_GLU
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <stdio.h>

static void APIENTRY
gl_arb_debug_callback(GLenum /* source */, GLenum /* type */, GLuint /* id */,
                      GLenum /* severity */, GLsizei /* length */,
                      const char *message, const void *userParam)
{
    FILE *log = reinterpret_cast<FILE *>(const_cast<void *>(userParam));
    fprintf(log, "GL message: %s\n", message);
}

int
main()
{

    FILE *log = fopen("log.txt", "a");
    int ret = 0;

    fprintf(log, "Program started\n");

    GLFWwindow *window;
    float aspect_ratio;
    int fb_width, fb_height;
    GLenum glew_err;

    /* Initialize the library */
    if (!glfwInit()) {
        fprintf(log, "GLFW init failed");
        ret = -1;
        goto err_init;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
        ret = -1;
        goto err_win;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glew_err = glewInit();
    if (glew_err != GLEW_OK) {
        fprintf(log, "GLEW Error: %s\n", glewGetErrorString(glew_err));
        ret = -1;
        goto err_win;
    }
    fprintf(log, "GLEW initialized\n");

    if (GLEW_ARB_debug_output) {
        fprintf(log, "GLEW_ARB_debug_output available\n");

        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB(gl_arb_debug_callback, log);
        glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                                 NULL, GL_TRUE);
    }

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        aspect_ratio = fb_width / (float) fb_height;
        glViewport(0, 0, fb_width, fb_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-aspect_ratio, aspect_ratio, -1.f, 1.f, 1.f, -1.f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
        glBegin(GL_TRIANGLES);
        glColor3f(1.f, 0.f, 0.f);
        glVertex3f(-0.6f, -0.4f, 0.f);
        glColor3f(0.f, 1.f, 0.f);
        glVertex3f(0.6f, -0.4f, 0.f);
        glColor3f(0.f, 0.f, 1.f);
        glVertex3f(0.f, 0.6f, 0.f);
        glEnd();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ret = 0;

err_win:
    glfwTerminate();

err_init:
    fclose(log);

    return ret;
}
