#include "glt/ShaderObject.hpp"

namespace glt {

ShaderObject::~ShaderObject() {
    glDeleteShader(handle);
    handle = 0;
}

} // namespace glt

