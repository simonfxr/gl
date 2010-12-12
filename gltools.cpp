#include <GL/glew.h>
#include <GL/glext.h>

#include "gltools.hpp"

#include <iostream>
#include <sstream>


namespace gltools {

std::string getErrorString(GLenum err) {

    switch (err) {
            
#define err_case(e) case e: return #e
        
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

        std::stringstream rep;
        rep << "Unknown OpenGL error [code = " << err << "]";
        return rep.str();
    }
}

bool printErrors(std::ostream& out) {
    bool was_error = false;
    
    for (GLenum err; (err = glGetError()) != GL_NO_ERROR; was_error = true)
        out << "OpenGL error occurred: " << getErrorString(err) << std::endl;
    
    return was_error;
}

void error(const char *msg, const char *file, int line, const char *func) {
    std::cerr << "ERROR in " << func << std::endl
              << " at " << file << ":" << line << std::endl
              << " message: " << msg << std::endl;
}

bool checkForGLError(const char *op, const char *file, int line, const char *func) {
    bool was_error = false;

    for (GLenum err; (err = glGetError()) != GL_NO_ERROR; was_error = true) {
        std::cerr << "OpenGL ERROR occurred: " << getErrorString(err) << std::endl
                  << "  in operation: " << op << std::endl
                  << "  in function: " << func << std::endl
                  << "  at " << file << ":" << line << std::endl;
    }

#ifdef GL_ARB_debug_output
#warning "GL_ARB_debug_output available"

    // PFNGLGETDEBUGMESSAGELOGARBPROC glGetDebugMessageLogARB = glXGetProcAddressARB("glGetDebugMessageLogARB");
    // if (glGetDebugMessageLogARB == 0) return was_error;

    GLsizei num_messages;
    glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES_ARB, &num_messages);

    for (GLsizei i = 0; i < num_messages; ++i) {
        
        GLsizei length;

        glGetIntegerv(GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB, &length);
        char *message = new char[length];
        GLuint id;
        GLsizei message_length;
        GLenum source, type, serverity;

        glGetDebugMessageLogARB(1, length, &source, &type, &id, &serverity, &message_length, message);

        std::cout << "OpenGL Debug Message: " << message << std::endl;

        delete [] message;
    }

#else
#warning "GL_ARB_debug_output not available"
#endif
    
    return was_error;
}

} // namespace gltools
