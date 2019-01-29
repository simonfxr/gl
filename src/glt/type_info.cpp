#include "glt/type_info.hpp"

#include "err/err.hpp"
#include "opengl.hpp"

namespace glt {

unsigned
toGLScalarType(ScalarType t)
{
    switch (t.value) {
    case ScalarType::I8:
        return GL_BYTE;
    case ScalarType::I16:
        return GL_SHORT;
    case ScalarType::I32:
        return GL_INT;
    case ScalarType::U8:
        return GL_UNSIGNED_BYTE;
    case ScalarType::U16:
        return GL_UNSIGNED_SHORT;
    case ScalarType::U32:
        return GL_UNSIGNED_INT;
    case ScalarType::F32:
        return GL_FLOAT;
    case ScalarType::F64:
        return GL_DOUBLE;
    }
    UNREACHABLE;
}

} // namespace glt
