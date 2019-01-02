#ifndef VEC3_TYPE
#define VEC3_TYPE

#include "math/real/type.hpp"

namespace math {

struct glvec3;

struct ATTRS(ATTR_PACKED) vec3_t
{
    static const defs::size_t size_t = 3;
    static const defs::size_t padded_size = 3;
    typedef real component_type;
    typedef component_type buffer[size_t];
    typedef glvec3 gl;

    constexpr MATH_FUNC real &operator[](defs::index_t) MUT_FUNC;
    constexpr MATH_FUNC real operator[](defs::index_t) const PURE_FUNC;

    real components[padded_size];
};

typedef vec3_t point3_t;

typedef vec3_t direction3_t; // a unit vector

typedef vec3_t normal3_t; // a unit vector, perdendicular to some surface

} // namespace math

#endif
