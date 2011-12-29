#ifndef GL_UTILS
#define GL_UTILS

#include "defs.hpp"
#include "opengl.hpp"

#include "err/err.hpp"
#include "sys/io/Stream.hpp"

#include <string>

#ifdef GLDEBUG
#define GL_CHECK(op) do {                                       \
        (op);                                                   \
        ::glt::checkForGLError(_CURRENT_LOCATION_OP(#op));    \
    } while (0)
#define GL_CHECK_ERRORS() ::glt::checkForGLError(_CURRENT_LOCATION)
#else
#define GL_CHECK(op) UNUSED(op)
#define GL_CHECK_ERRORS() UNUSED(0)
#endif

namespace glt {

bool printOpenGLCalls();

void printOpenGLCalls(bool);

std::string getGLErrorString(GLenum err);

bool printGLErrors(sys::io::OutStream& out);

bool checkForGLError(const err::Location& loc);

bool isExtensionSupported(const char *extension);

bool initDebug();

// in kilobytes
struct GLMemFree {
    defs::size freeVBO;
    defs::size freeTexture;
    defs::size freeRenderbuffer;
};

struct GLMemInfo {
    GLMemFree current;
    GLMemFree initial;
};

bool initMemInfo();

bool getMemInfo(GLMemInfo*);

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
