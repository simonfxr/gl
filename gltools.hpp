#ifndef GL_UTILS
#define GL_UTILS

#include <GL/gl.h>
#include <string>
#include <ostream>

namespace gltools {

const std::string& getErrorString(GLenum err);
bool printErrors(std::ostream& out);

}

#endif
