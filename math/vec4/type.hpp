#ifndef VEC4_TYPE
#define VEC4_TYPE

#include "math/defs.hpp"

namespace math {

struct vec4_t {
    
    union {
        struct {
            float x, y, z, w;
        };

        struct {
            float r, g, b, a;
        };

        struct {
            float s, t, p, q;
        };
            
        float components[4];
        
    } __attribute__((aligned(16)));

#ifndef MATH_INLINE
    float& MUT_FUNC operator[](unsigned long i);
    float  PURE_FUNC operator[](unsigned long i) const;
#else
    
    float& MUT_FUNC operator[](unsigned long i) {
        return components[i];
    }

    float PURE_FUNC operator[](unsigned long i) const {
        return components[i];
    }

#endif
};

typedef vec4_t point4_t;

} // namespace math

#endif
