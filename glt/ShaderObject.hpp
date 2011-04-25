#ifndef GLT_SHADER_OBJECT_HPP
#define GLT_SHADER_OBJECT_HPP

#include "opengl.h"

namespace glt {

struct ShaderObject {
    GLuint handle;
    ShaderObject(GLuint h = 0) : handle(h) {}
    ~ShaderObject();
};

} // namespace glt

#endif
