#ifndef VEC3_TYPE
#define VEC3_TYPE

#include "math/defs.hpp"

namespace math {

struct vec3_t {
    
    union {
        struct {
            float x, y, z;
        };

        struct {
            float r, g, b;
        };

        struct {
            float s, t, p;
        };
        
        float components[3];
    };

    float& operator[](unsigned long i) MUT_FUNC;
    float operator[](unsigned long i) const PURE_FUNC;
};

typedef vec3_t point3_t;

typedef vec3_t direction3_t; // a unit vector

typedef vec3_t normal3_t; // a unit vector, perdendicular to a plane

} // namespace math

#endif
