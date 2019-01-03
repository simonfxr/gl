#ifndef MATH_VEC2_TYPE
#define MATH_VEC2_TYPE

#include "math/real/type.hpp"

namespace math {

struct glvec2;

struct  vec2_t
{
    static const size_t size = 2;
    static const size_t padded_size = 2;
    typedef real component_type;
    typedef component_type buffer[size];
    typedef glvec2 gl;

    inline constexpr real &operator[](size_t i) { return components[i]; }

    inline constexpr real operator[](size_t i) const { return components[i]; }

    real components[padded_size];
};

typedef vec2_t point2_t;

typedef vec2_t direction2_t;

typedef vec2_t normal2_t;

} // namespace math

#endif
