#ifndef MATH_VEC2_TYPE
#define MATH_VEC2_TYPE

#include "math/real/type.hpp"

namespace math {

struct glvec2;

struct ATTRS(ATTR_PACKED) vec2_t
{
    static const defs::size_t size_t = 2;
    static const defs::size_t padded_size = 2;
    typedef real component_type;
    typedef component_type buffer[size_t];
    typedef glvec2 gl;

    constexpr MATH_FUNC real &operator[](defs::index_t) MUT_FUNC;
    constexpr MATH_FUNC real operator[](defs::index_t) const PURE_FUNC;

    real components[padded_size];
};

typedef vec2_t point2_t;

typedef vec2_t direction2_t;

typedef vec2_t normal2_t;

} // namespace math

#endif
