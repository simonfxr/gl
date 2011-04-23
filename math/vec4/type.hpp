#ifndef VEC4_TYPE
#define VEC4_TYPE

#include "math/defs.hpp"

#if MATH_SSE(2, 0)
#include <xmmintrin.h>
#endif

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

#if MATH_SSE(2, 0)
        __m128 packed;
#endif

    } ATTRS(ATTR_ALIGNED(16));

    float& operator[](unsigned long i) MUT_FUNC;
    float operator[](unsigned long i) const PURE_FUNC;
};

typedef vec4_t point4_t;

typedef vec4_t ATTRS(ATTR_ALIGNED(16)) aligned_vec4_t;

typedef aligned_vec4_t aligned_point4_t;

} // namespace math

#endif
