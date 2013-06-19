#ifndef GL_UTILS
#define GL_UTILS

#include "glt/conf.hpp"
#include "opengl.hpp"

#include "err/err.hpp"
#include "sys/io/Stream.hpp"

#include <string>

#ifdef GLDEBUG
#define GL_TRACE(msg) do {                                              \
        ::glt::printGLTrace(_CURRENT_LOCATION_OP(msg));                 \
    } while(0)
#define GL_CHECK(op) do {                                       \
        (op);                                                   \
        ::glt::checkForGLError(_CURRENT_LOCATION_OP(#op));      \
    } while (0)
#define GL_CHECK_ERRORS() ::glt::checkForGLError(_CURRENT_LOCATION)
#else
#define GL_CHECK(op) UNUSED(op)
#define GL_CHECK_ERRORS() UNUSED(0)
#define GL_TRACE(loc)
#endif

#define GL_CALL(fn, ...) GL_CHECK(fn(__VA_ARGS__))
#define GL_ASSIGN_CALL(var, fn, ...) GL_CHECK(var = fn(__VA_ARGS__))

#define GL_CALL_NO_CHECK(fn, ...)

namespace glt {

GLT_API bool printOpenGLCalls();

GLT_API void printOpenGLCalls(bool);

GLT_API void printGLTrace(const err::Location& loc);

GLT_API std::string getGLErrorString(GLenum err);

GLT_API bool printGLErrors(sys::io::OutStream& out);

GLT_API bool checkForGLError(const err::Location& loc);

GLT_API bool isExtensionSupported(const char *extension);

GLT_API bool initDebug();

namespace glvendor {

enum OpenGLVendor {
    Unknown,
    Nvidia,
    ATI,
    Intel
};

} // namespace glvendor

typedef glvendor::OpenGLVendor OpenGLVendor;

GLT_API void ignoreDebugMessage(OpenGLVendor, GLuint);

struct GLT_API GLMemInfoATI {

    struct GLMemFree {
        // in kb
        defs::size freeVBO;
        defs::size freeTexture;
        defs::size freeRenderbuffer;
    };

    GLMemFree current;
    GLMemFree initial;

    static bool init();
    static bool info(GLMemInfoATI *);
};

struct GLT_API GLMemInfoNV {

    // in kb
    defs::size total;
    defs::size total_dedicated;
    defs::size current;
    defs::size evicted;
    defs::size num_evictions;

    static bool init();
    static bool info(GLMemInfoNV *);
};

inline const GLubyte *gl_str(const char *str) {
    return reinterpret_cast<const GLubyte *>(str);
}

inline const char *gl_unstr(const GLubyte *glstr) {
    return reinterpret_cast<const char *>(glstr);
}

inline GLboolean gl_bool(bool b) {
    return b ? GLboolean(GL_TRUE) : GLboolean(GL_FALSE);
}

inline bool gl_unbool(GLboolean b) {
    return b != GLboolean(GL_FALSE) ? true : false;
}

inline bool gl_unbool(GLint b) {
    return b != GLint(GL_FALSE) ? true : false;
}

} // namespace glt

#endif
