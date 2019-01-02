#include "math/glvec/type.hpp"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

#include "math/mat2.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

namespace math {

#define DEF_GLVEC_CONSTR(gl_T, T, constr)                                      \
    MATH_INLINE_SPEC gl_T::gl_T(const T &v)                                    \
    {                                                                          \
        T::buffer buf;                                                         \
        load(buf, v);                                                          \
        for (defs::index_t i = 0; i < T::size_t; ++i)                              \
            buffer[i] = float(buf[i]);                                         \
    }                                                                          \
    MATH_INLINE_SPEC gl_T::operator T() const                                  \
    {                                                                          \
        T::buffer buf;                                                         \
        for (defs::index_t i = 0; i < T::size_t; ++i)                              \
            buf[i] = T::component_type(buffer[i]);                             \
        return constr(buf);                                                    \
    }

DEF_GLVEC_CONSTR(glvec2, vec2_t, vec2);
DEF_GLVEC_CONSTR(glvec3, vec3_t, vec3);
DEF_GLVEC_CONSTR(glvec4, vec4_t, vec4);

DEF_GLVEC_CONSTR(glmat2, mat2_t, mat2);
DEF_GLVEC_CONSTR(glmat3, mat3_t, mat3);
DEF_GLVEC_CONSTR(glmat4, mat4_t, mat4);

} // namespace math
