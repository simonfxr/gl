#ifndef GL_UTILS
#define GL_UTILS

#include "defs.h"
#include "opengl.h"

#include <string>
#include <ostream>

#ifdef GLDEBUG
#define GL_CHECK(op) do {                                               \
        (op);                                                           \
        glt::checkForGLError(#op, __FILE__, __LINE__, __func__);    \
    } while (0)
#else
#define GL_CHECK(op) op
#endif

namespace glt {

std::string getGLErrorString(GLenum err);

bool printGLErrors(std::ostream& out);

bool checkForGLError(const char *op, const char *file, int line, const char *func);

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
