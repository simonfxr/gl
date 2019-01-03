#ifndef VEC4_TYPE
#define VEC4_TYPE

#include "math/real/type.hpp"

namespace math {

struct glvec4;

struct vec4_t
{
    static const size_t size = 4;
    static const size_t padded_size = 4;
    typedef real component_type;
    typedef component_type buffer[size];
    typedef glvec4 gl;

    real components[padded_size];

    constexpr inline real &operator[](size_t i) { return components[i]; }

    constexpr inline real operator[](size_t i) const { return components[i]; }
};

typedef vec4_t point4_t;

typedef vec4_t HU_ALIGN(16) aligned_vec4_t;

typedef aligned_vec4_t aligned_point4_t;

} // namespace math

#endif
