#include "math/glvec/type.hpp"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

#include "math/mat2.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

namespace math {

#define DEF_GLVEC_CONSTR(gl_T, T) \
    MATH_INLINE_SPEC gl_T::gl_T(const T& v) {           \
        T::buffer buf;                                  \
        load(buf, v);                                   \
        for (defs::index i = 0; i < T::size; ++i)       \
            buffer[i] = float(buf[i]);                  \
    }

DEF_GLVEC_CONSTR(glvec2, vec2_t);
DEF_GLVEC_CONSTR(glvec3, vec3_t);
DEF_GLVEC_CONSTR(glvec4, vec4_t);

DEF_GLVEC_CONSTR(glmat2, mat2_t);
DEF_GLVEC_CONSTR(glmat3, mat3_t);
DEF_GLVEC_CONSTR(glmat4, mat4_t);

} // namespace math
