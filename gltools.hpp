#ifndef GL_UTILS
#define GL_UTILS

#include <GL/gl.h>
#include <string>
#include <ostream>

namespace gltools {

std::string getErrorString(GLenum err);

bool printErrors(std::ostream& out);

void error(const char *msg, const char *file, int line, const char *func);

} // namespace gltools

#endif
