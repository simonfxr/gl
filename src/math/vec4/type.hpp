#ifndef VEC4_TYPE
#define VEC4_TYPE

#include "math/real/type.hpp"

namespace math {

struct glvec4;

struct ATTRS(ATTR_PACKED) vec4_t
{
    static const size_t size = 4;
    static const size_t padded_size = 4;
    typedef real component_type;
    typedef component_type buffer[size];
    typedef glvec4 gl;

    real components[padded_size];

    constexpr MATH_FUNC real &operator[](size_t) MUT_FUNC;
    constexpr MATH_FUNC real operator[](size_t) const PURE_FUNC;
};

typedef vec4_t point4_t;

typedef vec4_t ATTRS(ATTR_ALIGNED(16)) aligned_vec4_t;

typedef aligned_vec4_t aligned_point4_t;

} // namespace math

#endif
