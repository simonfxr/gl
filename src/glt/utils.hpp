#ifndef GL_UTILS
#define GL_UTILS

#include "defs.h"
#include "opengl.h"

#include "err/err.hpp"

#include <string>
#include <ostream>

#ifdef GLDEBUG
#define GL_CHECK(op) do {                                       \
        (op);                                                   \
        ::glt::checkForGLError(_CURRENT_LOCATION_OP(AS_STRING(op)));    \
    } while (0)
#define GL_CHECK_ERRORS() ::glt::checkForGLError(_CURRENT_LOCATION)
#else
#define GL_CHECK(op) UNUSED(op)
#define GL_CHECK_ERRORS() UNUSED(0)
#endif

namespace glt {

std::string getGLErrorString(GLenum err);

bool printGLErrors(std::ostream& out);

bool checkForGLError(const err::Location& loc);

bool isExtensionSupported(const char *extension);

bool initDebug();

inline const GLubyte *gl_str(const char *str) {
    return reinterpret_cast<const GLubyte *>(str);
}

inline const char *gl_unstr(const GLubyte *glstr) {
    return reinterpret_cast<const char *>(glstr);
}

} // namespace glt

#endif
