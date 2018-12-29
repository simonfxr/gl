#ifndef MATH_VEC2_TYPE
#define MATH_VEC2_TYPE

#include "math/real/type.hpp"

namespace math {

struct glvec2;

struct ATTRS(ATTR_PACKED) vec2_t
{
    static const defs::size size = 2;
    static const defs::size padded_size = 2;
    typedef real component_type;
    typedef component_type buffer[size];
    typedef glvec2 gl;

    MATH_FUNC real &operator[](defs::index) MUT_FUNC;
    MATH_FUNC real operator[](defs::index) const PURE_FUNC;

    real components[padded_size];
};

typedef vec2_t point2_t;

typedef vec2_t direction2_t;

typedef vec2_t normal2_t;

} // namespace math

#endif
