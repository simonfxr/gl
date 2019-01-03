#ifndef VEC3_TYPE
#define VEC3_TYPE

#include "math/real/type.hpp"

namespace math {

struct glvec3;

struct ATTRS(ATTR_PACKED) vec3_t
{
    static const size_t size = 3;
    static const size_t padded_size = 3;
    typedef real component_type;
    typedef component_type buffer[size];
    typedef glvec3 gl;

    constexpr inline real &operator[](size_t i) { return components[i]; }

    constexpr inline real operator[](size_t i) const { return components[i]; }

    real components[padded_size];
};

typedef vec3_t point3_t;

typedef vec3_t direction3_t; // a unit vector

typedef vec3_t normal3_t; // a unit vector, perdendicular to some surface

} // namespace math

#endif
