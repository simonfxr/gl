#ifndef GL_UTILS
#define GL_UTILS

#include "opengl.h"

#include <string>
#include <ostream>

#ifdef DEBUG
#define GL_CHECK(op) do {                                               \
        (op);                                                           \
        glt::checkForGLError(#op, __FILE__, __LINE__, __func__);    \
    } while (0)
#else
#define GL_CHECK(op) op
#endif

namespace glt {

std::string getErrorString(GLenum err);

bool printErrors(std::ostream& out);

void error(const char *msg, const char *file, int line, const char *func);

void error_once(const char *msg, const char *file, int line, const char *func);

void fatal_error(const char *msg, const char *file, int line, const char *func) __attribute__((noreturn));

bool checkForGLError(const char *op, const char *file, int line, const char *func);

bool isExtensionSupported(const char *extension);

bool initDebug();

// if filename doesnt have a trailing / then use dirname of filename
void setWorkingDirectory(const char *filename);

inline const GLubyte *gl_str(const char *str) {
    return reinterpret_cast<const GLubyte *>(str);
}

inline const char *gl_unstr(const GLubyte *glstr) {
    return reinterpret_cast<const char *>(glstr);
}

} // namespace glt

#endif
