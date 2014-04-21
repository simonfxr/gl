#ifndef MATH_GLVEC_HPP
#define MATH_GLVEC_HPP

#include "math/vec2/type.hpp"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"

#include "math/mat2/type.hpp"
#include "math/mat3/type.hpp"
#include "math/mat4/type.hpp"

namespace math {

#define DEF_GLVEC(gl_T, T)                              \
    struct gl_T {                                       \
    float buffer[T::size];                              \
    gl_T() = default;                                   \
    gl_T(const T& v);                                   \
    };

DEF_GLVEC(glvec2, vec2_t);
DEF_GLVEC(glvec3, vec3_t);
DEF_GLVEC(glvec4, vec4_t);

DEF_GLVEC(glmat2, mat2_t);
DEF_GLVEC(glmat3, mat3_t);
DEF_GLVEC(glmat4, mat4_t);

} // namespace math

#endif
