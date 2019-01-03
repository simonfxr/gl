#ifndef MATH_VEC2_TYPE
#define MATH_VEC2_TYPE

#include "math/real/type.hpp"

namespace math {

struct glvec2;

struct ATTRS(ATTR_PACKED) vec2_t
{
    static const size_t size = 2;
    static const size_t padded_size = 2;
    typedef real component_type;
    typedef component_type buffer[size];
    typedef glvec2 gl;

    MATH_FUNC constexpr real &operator[](size_t) MUT_FUNC;
    MATH_FUNC constexpr real operator[](size_t) const PURE_FUNC;

    real components[padded_size];
};

typedef vec2_t point2_t;

typedef vec2_t direction2_t;

typedef vec2_t normal2_t;

} // namespace math

#endif
