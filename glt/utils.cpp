#include <iostream>
#include <sstream>
#include <cstring>

#include "defs.h"
#include "opengl.h"

#include "glt/utils.hpp"
#include "error/error.hpp"

namespace glt {

std::string getGLErrorString(GLenum err) {

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

        std::ostringstream rep;
        rep << "Unknown OpenGL error [code = " << err << "]";
        return rep.str();
    }
}

bool printGLErrors(std::ostream& out) {
    bool was_error = false;
    
    for (GLenum err; (err = glGetError()) != GL_NO_ERROR; was_error = true)
        out << "OpenGL error occurred: " << getGLErrorString(err) << std::endl;
    
    return was_error;
}

bool isExtensionSupported(const char *extension) {
    GLint n;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    
    for(GLint i = 0; i < n; i++)
        if(strcmp(extension, gl_unstr(glGetStringi(GL_EXTENSIONS, i))) == 0)
            return true;

    return false;
}

namespace {

struct GLDebug {
    virtual ~GLDebug() {}
    virtual void printDebugMessages(const err::Location& loc) = 0;
};

struct NODebug : public GLDebug {
    virtual void printDebugMessages(const err::Location& loc) {
        UNUSED(loc);
    }
};

struct ARBDebug : public GLDebug {
    GLsizei message_buffer_length;
    char *message_buffer;

    ~ARBDebug();
    
    static GLDebug* init();
    virtual void printDebugMessages(const err::Location& loc);
};

ARBDebug::~ARBDebug() {
    delete[] message_buffer;
}

GLDebug *ARBDebug::init() {
    glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        return 0;

    GLsizei max_len;
    glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH_ARB, &max_len);
    
    ARBDebug *dbg = new ARBDebug();
    dbg->message_buffer_length = max_len;
    dbg->message_buffer = new char[max_len];
    return dbg;
}

void ARBDebug::printDebugMessages(const err::Location& loc) {

    GLenum source, type, id, severity;
    GLsizei length;

    while (glGetDebugMessageLogARB(1, message_buffer_length,
                                   &source, &type, &id, &severity,
                                   &length, message_buffer) > 0) {

#define sym_case(v, c) case c: v = #c; break

        const char *ssrc = "unknown";
        switch (source) {
            sym_case(ssrc, GL_DEBUG_SOURCE_API_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_SHADER_COMPILER_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_THIRD_PARTY_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_APPLICATION_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_OTHER_ARB);
        }

        const char *stype = "unknown";
        switch (type) {
            sym_case(stype, GL_DEBUG_TYPE_ERROR_ARB);
            sym_case(stype, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB);
            sym_case(stype, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB);
            sym_case(stype, GL_DEBUG_TYPE_PORTABILITY_ARB);
            sym_case(stype, GL_DEBUG_TYPE_PERFORMANCE_ARB);
            sym_case(stype, GL_DEBUG_TYPE_OTHER_ARB);
        }

        const char *ssev = "unknown";
        switch (severity) {
            sym_case(ssev, GL_DEBUG_SEVERITY_HIGH_ARB);
            sym_case(ssev, GL_DEBUG_SEVERITY_MEDIUM_ARB);
            sym_case(ssev, GL_DEBUG_SEVERITY_LOW_ARB);
        }
        
#undef sym_case

        std::stringstream mesg;
        mesg << "source: " << ssrc << ", severity: " << ssev
             << ", type: " << stype << ", id: " << id
             << "  message: " << message_buffer;

        err::printError(std::cerr, "OpenGL DEBUG", loc, err::Error, mesg.str());               }
}

struct AMDDebug : public GLDebug {

    GLsizei message_buffer_length;
    char *message_buffer;

    ~AMDDebug();
    
    static GLDebug* init();
    virtual void printDebugMessages(const err::Location& loc);
};

AMDDebug::~AMDDebug() {
    delete[] message_buffer;
}

GLDebug *AMDDebug::init() {

    if (!GLEW_AMD_debug_output)
        return 0;

    glDebugMessageEnableAMD(0, 0, 0, NULL, GL_TRUE);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        return 0;

    GLsizei max_len;
    glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH_AMD, &max_len);
    
    AMDDebug *dbg = new AMDDebug();
    dbg->message_buffer_length = max_len;
    dbg->message_buffer = new char[max_len];
        
    return dbg;
}

void AMDDebug::printDebugMessages(const err::Location& loc) {

    GLenum category;
    GLuint severity, id;
    GLsizei length;
        
    while (glGetDebugMessageLogAMD(1, message_buffer_length,
                                   &category, &severity, &id, &length,
                                   message_buffer) > 0) {

#define sym_case(v, c) case c: v = #c; break
        
        const char *scat = "unknown";
        switch (category) {
            sym_case(scat, GL_DEBUG_CATEGORY_API_ERROR_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_DEPRECATION_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_PERFORMANCE_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_APPLICATION_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_OTHER_AMD);
        }

        const char *ssev = "unknown";
        switch (severity) {
            sym_case(ssev, GL_DEBUG_SEVERITY_HIGH_AMD);
            sym_case(ssev, GL_DEBUG_SEVERITY_MEDIUM_AMD);
            sym_case(ssev, GL_DEBUG_SEVERITY_LOW_AMD);
        }

#undef sym_case

        std::stringstream mesg;
        mesg << "category: " << scat << ", severity: " << ssev
             << ", id: " << id << "  message: " << message_buffer;

        err::printError(std::cerr, "OpenGL DEBUG", loc, err::Error, mesg.str());
    }
}

GLDebug *glDebug = new NODebug();

} // namespace anon

void printGLError(const err::Location& loc, GLenum err) {
    err::printError(std::cerr, "OpenGL Error", loc, err::Error, getGLErrorString(err));
}

bool checkForGLError(const err::Location& loc) {
    bool was_error = false;

    for (GLenum err; (err = glGetError()) != GL_NO_ERROR; was_error = true)
        printGLError(loc, err);

    glDebug->printDebugMessages(loc);
    
    return was_error;
}

bool initDebug() {

    GLDebug *dbg = 0;
    const char *debug_impl = 0;

    glewInit();

    if (isExtensionSupported("GL_ARB_debug_output")) {
        debug_impl = "GL_ARB_debug_output";
        dbg = ARBDebug::init();
    }
    
    if (dbg == 0 && isExtensionSupported("GL_AMD_debug_output")) {
        debug_impl = "GL_AMD_debug_output";
        dbg = AMDDebug::init();
    }

    bool initialized;

    if (dbg == 0) {
        std::cerr << "couldnt initialize Debug Output, no debug implementaion available" << std::endl;
        dbg = new NODebug();
        initialized = false;
    } else {
        std::cerr << "initialized Debug Output using " << debug_impl << std::endl;
        initialized = true;
    }

    delete glDebug;
    glDebug = dbg;
    return initialized;
}

} // namespace glt
