#ifndef VEC4_RAW_HPP
#define VEC4_RAW_HPP

#ifdef USE_SSE
#include <xmmintrin.h>
#include <smmintrin.h>
#endif

namespace v4 {

    struct v4 {
#ifdef USE_SSE
        __m128 packed;
#else
        float packed[4];
#endif
    } __attribute__((aligned(16)));
    
    namespace {
        
        v4 add(const v4 *a, const v4 *b) {
#ifdef USE_SSE
            v4 r = { _mm_add_ps(a->packed, b->packed) };
#else
            v4 r;
            for (int i = 0; i < 4; ++i)
                r.packed[i] = a->packed[i] + b->packed[i];
#endif
            return r;
        }

        float dot(const v4 *a, const v4 *b) {
#ifdef USE_SSE
            return _mm_cvtss_f32(_mm_dp_ps(a->packed, b->packed, 0xF1));
#else
            float x = 0.f;
            for (int i = 0; i < 4; ++i)
                x += a->packed[i] * b->packed[i];
            return x;
#endif        
        }
    }
}

#endif
