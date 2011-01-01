#include <iostream>
#include <sstream>
#include <cstdlib>

#include <GL/glew.h>
#include <GL/glxew.h>

#include "gltools.hpp"
#include "defs.h"

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
    std::cerr << "ERROR in " << func << "()" << std::endl
              << " at " << file << ":" << line << std::endl
              << " message: " << msg << std::endl;
}

void fatal_error(const char *msg, const char *file, int line, const char *func) {
    error(msg, file, line, func);
    exit(2);
}

struct DebugLocation {
    const char *op;
    const char *file;
    int line;
    const char *func;
};

namespace {

#ifdef GLDEBUG

struct GLDebug {
    virtual ~GLDebug() {}
    virtual void printDebugMessages(const DebugLocation& loc) = 0;
};

struct NODebug : public GLDebug {
    virtual void printDebugMessages(const DebugLocation& loc) {
        UNUSED(loc);
    }
};

struct ARBDebug : public GLDebug {

    typedef void (GLAPIENTRY *glDebugMessageControlARBf_t)(GLenum source,
                                                           GLenum type,
                                                           GLenum severity,
                                                           GLsizei count,
                                                           const GLuint* ids,
                                                           GLboolean enabled);

    typedef GLuint (GLAPIENTRY * glGetDebugMessageLogARBf_t)(GLuint count,
                                                             GLsizei bufsize,
                                                             GLenum* sources,
                                                             GLenum* types,
                                                             GLuint* ids,
                                                             GLenum* severities,
                                                             GLsizei* lengths, 
                                                             char* message);

    static const GLenum MAX_DEBUG_MESSAGE_LENGTH_ARB          = 0x9143;

    static const GLenum DEBUG_SEVERITY_HIGH_ARB               = 0x9146;
    static const GLenum DEBUG_SEVERITY_MEDIUM_ARB             = 0x9147;
    static const GLenum DEBUG_SEVERITY_LOW_ARB                = 0x9148;

    static const GLenum DEBUG_SOURCE_API_ARB                  = 0x8246;
    static const GLenum DEBUG_SOURCE_WINDOW_SYSTEM_ARB        = 0x8247;
    static const GLenum DEBUG_SOURCE_SHADER_COMPILER_ARB      = 0x8248;
    static const GLenum DEBUG_SOURCE_THIRD_PARTY_ARB          = 0x8249;
    static const GLenum DEBUG_SOURCE_APPLICATION_ARB          = 0x825A;
    static const GLenum DEBUG_SOURCE_OTHER_ARB                = 0x825B;

    static const GLenum DEBUG_TYPE_ERROR_ARB                  = 0x824C;
    static const GLenum DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB    = 0x824D;
    static const GLenum DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB     = 0x824E;
    static const GLenum DEBUG_TYPE_PORTABILITY_ARB            = 0x824F;
    static const GLenum DEBUG_TYPE_PERFORMANCE_ARB            = 0x8250;
    static const GLenum DEBUG_TYPE_OTHER_ARB                  = 0x8251;

    glGetDebugMessageLogARBf_t glGetDebugMessageLogARB;
    GLsizei message_buffer_length;
    char *message_buffer;

    ~ARBDebug();
    
    static GLDebug* init();
    virtual void printDebugMessages(const DebugLocation& loc);
};

ARBDebug::~ARBDebug() {
    delete[] message_buffer;
}

GLDebug *ARBDebug::init() {

    glDebugMessageControlARBf_t glDebugMessageEnableAMD
        = (glDebugMessageControlARBf_t) glXGetProcAddress((const GLubyte *) "glDebugMessageControlARB");
    
    glDebugMessageEnableAMD(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL ERROR occurred: " << getErrorString(err) << std::endl
                  << "  couldnt initialize debug functionality, maybe we dont have a debug context?" << std::endl;
            
        return new NODebug();
    }

    GLsizei max_len;
    glGetIntegerv(MAX_DEBUG_MESSAGE_LENGTH_ARB, &max_len);
    
    ARBDebug *dbg = new ARBDebug();
    dbg->message_buffer_length = max_len;
    dbg->message_buffer = new char[max_len];
    dbg->glGetDebugMessageLogARB
        = (glGetDebugMessageLogARBf_t) glXGetProcAddress((const GLubyte *) "glGetDebugMessageLogARB");
        
    return dbg;
}

void ARBDebug::printDebugMessages(const DebugLocation& loc) {

    GLenum source, type, id, serverity;
    GLsizei length;

    while (glGetDebugMessageLogARB(1, message_buffer_length,
                                   &source, &type, &id, &serverity,
                                   &length, message_buffer) > 0) {

#define sym_case(v, c) case c: v = #c; break

        const char *ssrc = "unknown";
        switch (source) {
            sym_case(ssrc, DEBUG_SOURCE_API_ARB);
            sym_case(ssrc, DEBUG_SOURCE_WINDOW_SYSTEM_ARB);
            sym_case(ssrc, DEBUG_SOURCE_SHADER_COMPILER_ARB);
            sym_case(ssrc, DEBUG_SOURCE_THIRD_PARTY_ARB);
            sym_case(ssrc, DEBUG_SOURCE_APPLICATION_ARB);
            sym_case(ssrc, DEBUG_SOURCE_OTHER_ARB);
        }

        const char *stype = "unknown";
        switch (type) {
            sym_case(stype, DEBUG_TYPE_ERROR_ARB);
            sym_case(stype, DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB);
            sym_case(stype, DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB);
            sym_case(stype, DEBUG_TYPE_PORTABILITY_ARB);
            sym_case(stype, DEBUG_TYPE_PERFORMANCE_ARB);
            sym_case(stype, DEBUG_TYPE_OTHER_ARB);
        }

        const char *ssev = "unknown";
        switch (serverity) {
            sym_case(ssev, DEBUG_SEVERITY_HIGH_ARB);
            sym_case(ssev, DEBUG_SEVERITY_MEDIUM_ARB);
            sym_case(ssev, DEBUG_SEVERITY_LOW_ARB);
        }
        
#undef sym_case

        std::cerr << "[OpenGL DEBUG] source: " << ssrc << ", severity: " << ssev << ", type: " << stype << ", id: " << id << std::endl
                  << "  in operation: " << loc.op << std::endl
                  << "  in function: " << loc.func << std::endl
                  << "  at " << loc.file << ":" << loc.line << std::endl
                  << "  message: " << message_buffer << std::endl;
    }
}

struct AMDDebug : public GLDebug {

    typedef void (GLAPIENTRY *glDebugMessageEnableAMDf_t)(GLenum category,
                                                          GLenum severity,
                                                          GLsizei count,
                                                          const GLuint* ids,
                                                          GLboolean enabled);

    typedef GLuint (GLAPIENTRY * glGetDebugMessageLogAMDf_t)(GLuint count,
                                                             GLsizei bufsize,
                                                             GLenum* categories,
                                                             GLuint* severities,
                                                             GLuint* ids,
                                                             GLsizei* lengths, 
                                                             char* message);

    static const GLenum MAX_DEBUG_MESSAGE_LENGTH_AMD          = 0x9143;

    static const GLenum DEBUG_SEVERITY_HIGH_AMD               = 0x9146;
    static const GLenum DEBUG_SEVERITY_MEDIUM_AMD             = 0x9147;
    static const GLenum DEBUG_SEVERITY_LOW_AMD                = 0x9148;

    static const GLenum DEBUG_CATEGORY_API_ERROR_AMD          = 0x9149;
    static const GLenum DEBUG_CATEGORY_WINDOW_SYSTEM_AMD      = 0x914A;
    static const GLenum DEBUG_CATEGORY_DEPRECATION_AMD        = 0x914B;
    static const GLenum DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD = 0x914C;
    static const GLenum DEBUG_CATEGORY_PERFORMANCE_AMD        = 0x914D;
    static const GLenum DEBUG_CATEGORY_SHADER_COMPILER_AMD    = 0x914E;
    static const GLenum DEBUG_CATEGORY_APPLICATION_AMD        = 0x914F;
    static const GLenum DEBUG_CATEGORY_OTHER_AMD              = 0x9150;

    glGetDebugMessageLogAMDf_t glGetDebugMessageLogAMD;
    GLsizei message_buffer_length;
    char *message_buffer;

    ~AMDDebug();
    
    static GLDebug* init();
    virtual void printDebugMessages(const DebugLocation& loc);
};

AMDDebug::~AMDDebug() {
    delete[] message_buffer;
}

GLDebug *AMDDebug::init() {

    glDebugMessageEnableAMDf_t glDebugMessageEnableAMD
        = (glDebugMessageEnableAMDf_t) glXGetProcAddress((const GLubyte *) "glDebugMessageEnableAMD");

    glDebugMessageEnableAMD(0, 0, 0, NULL, GL_TRUE);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL ERROR occurred: " << getErrorString(err) << std::endl
                  << "  couldnt initialize debug functionality, maybe we dont have a debug context?" << std::endl;
            
        return new NODebug();
    }

    GLsizei max_len;
    glGetIntegerv(MAX_DEBUG_MESSAGE_LENGTH_AMD, &max_len);
    
    AMDDebug *dbg = new AMDDebug();
    dbg->message_buffer_length = max_len;
    dbg->message_buffer = new char[max_len];
    dbg->glGetDebugMessageLogAMD
        = (glGetDebugMessageLogAMDf_t) glXGetProcAddress((const GLubyte *) "glGetDebugMessageLogAMD");
        
    return dbg;
}

void AMDDebug::printDebugMessages(const DebugLocation& loc) {

    GLenum category;
    GLuint severity, id;
    GLsizei length;
        
    while (glGetDebugMessageLogAMD(1, message_buffer_length,
                                   &category, &severity, &id, &length,
                                   message_buffer) > 0) {

#define sym_case(v, c) case c: v = #c; break
        
        const char *scat = "unknown";
        switch (category) {
            sym_case(scat, DEBUG_CATEGORY_API_ERROR_AMD);
            sym_case(scat, DEBUG_CATEGORY_WINDOW_SYSTEM_AMD);
            sym_case(scat, DEBUG_CATEGORY_DEPRECATION_AMD);
            sym_case(scat, DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD);
            sym_case(scat, DEBUG_CATEGORY_PERFORMANCE_AMD);
            sym_case(scat, DEBUG_CATEGORY_SHADER_COMPILER_AMD);
            sym_case(scat, DEBUG_CATEGORY_APPLICATION_AMD);
            sym_case(scat, DEBUG_CATEGORY_OTHER_AMD);
        }

        const char *ssev = "unknown";
        switch (severity) {
            sym_case(ssev, DEBUG_SEVERITY_HIGH_AMD);
            sym_case(ssev, DEBUG_SEVERITY_MEDIUM_AMD);
            sym_case(ssev, DEBUG_SEVERITY_LOW_AMD);
        }

#undef sym_case
        
        std::cerr << "[OpenGL DEBUG] category: " << scat << ", serverity: " << ssev << ", id: " << id << std::endl
                  << "  in operation: " << loc.op << std::endl
                  << "  in function: " << loc.func << std::endl
                  << "  at " << loc.file << ":" << loc.line << std::endl
                  << "  message: " << message_buffer << std::endl;
    }
}

GLDebug *glDebug = 0;

GLDebug* initDebug() {
    if (glewIsExtensionSupported("GL_ARB_debug_output"))
        return ARBDebug::init();
    else if (glewIsExtensionSupported("GL_AMD_debug_output"))
        return AMDDebug::init();
    else {
        std::cerr << "no debug output available" << std::endl;
        return new NODebug();
    }
}

#endif // GLDEBUG

} // namespace anon

void printGLError(const DebugLocation& loc, GLenum err) {
    std::cerr << "OpenGL ERROR: " << getErrorString(err) << std::endl
              << "  in operation: " << loc.op << std::endl
              << "  in function: " << loc.func << std::endl
              << "  at " << loc.file << ":" << loc.line << std::endl;
}

bool checkForGLError(const char *op, const char *file, int line, const char *func) {
    bool was_error = false;

    DebugLocation loc;
    loc.op = op; loc.file = file; loc.line = line; loc.func = func;

    for (GLenum err; (err = glGetError()) != GL_NO_ERROR; was_error = true)
        printGLError(loc, err);

#ifdef GLDEBUG

    if (glDebug == 0)
        glDebug = initDebug();

    glDebug->printDebugMessages(loc);

#endif
    
    return was_error;
}

} // namespace gltools
