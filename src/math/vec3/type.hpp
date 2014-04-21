#ifndef VEC3_TYPE
#define VEC3_TYPE

#include "math/real/type.hpp"

namespace math {

struct glvec3;

struct ATTRS(ATTR_PACKED) vec3_t {
    static const defs::size size = 3;
    static const defs::size padded_size = 3;
    typedef real component_type;
    typedef component_type buffer[size];
    typedef glvec3 gl;

    MATH_FUNC real& operator[](defs::index) MUT_FUNC;
    MATH_FUNC real operator[](defs::index) const PURE_FUNC;
    
    real components[padded_size];
};

typedef vec3_t point3_t;

typedef vec3_t direction3_t; // a unit vector

typedef vec3_t normal3_t; // a unit vector, perdendicular to some surface

} // namespace math

#endif
