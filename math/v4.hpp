#ifndef V4_HPP
#define V4_HPP

#ifdef USE_SSE
#include <xmmintrin.h>
#endif

#if !defined(USE_SSE41) && defined(USE_SSE) && defined(__SSE4_1__)
#define USE_SSE41
#endif

#ifdef USE_SSE41
#include <smmintrin.h>
#endif

#define V4API __attribute__((always_inline, unused))

namespace v4 {

    struct v4 {
#ifdef USE_SSE
        __m128 packed;
#else
        float packed[4];
#endif
    } __attribute__((aligned(16)));
    
    namespace {

        v4 V4API make(float a, float b, float c, float d) {
#ifdef USE_SSE
            return (v4) { _mm_set_ps(a, b, c, d) };
#else
            return (v4) { { a, b, c, d } };
#endif
        }

        v4 V4API make3(float a, float b, float c) {
#ifdef USE_SSE
            return (v4) { _mm_set_ps(a, b, c, 0.f) };
#else
            return (v4) { { a, b, c, 0.f } };
#endif
        }
        
#ifdef USE_SSE
#define def_accessor(name, i)                           \
        float V4API name(v4 v) {                        \
            float flat[4] __attribute__((aligned(16))); \
                                                        \
            _mm_store_ps(flat, v.packed);               \
            return flat[i];                             \
        }
#else
#define def_accessor(name, i)                   \
        float V4API name(v4 v) {                \
            return v.packed[i];                 \
        }
#endif

        def_accessor(v4a, 0)
        def_accessor(v4b, 1)
        def_accessor(v4c, 2)
        def_accessor(v4d, 3)

#undef def_accessor

#define lift_op(r, in1, in2, len, op) do {                              \
            for (int __i = 0; __i < (len); ++__i)                       \
                (r).packed[__i] = (in1).packed[__i] op (in2).packed[__i]; \
        } while (0)
        
#define lift_op3(r, in1, in2, op) do {          \
            (r).packed[3] = 0.f;                \
            lift_op(r, in1, in2, 3, op);        \
        } while (0)
    
        v4 V4API add(v4 a, v4 b) {
#ifdef USE_SSE
            return (v4) { _mm_add_ps(a.packed, b.packed) };
#else
            v4 r; lift_op(r, a, b, 4, +); return r;
#endif
        }

        v4 V4API add3(v4 a, v4 b) {
#ifdef USE_SSE
            return (v4) { _mm_add_ps(a.packed, b.packed) };
#else
            v4 r; lift_op3(r, a, b, +); return r;
#endif
        }

        v4 V4API sub(v4 a, v4 b) {
#ifdef USE_SSE
            return (v4) { _mm_sub_ps(a.packed, b.packed) };
#else
            v4 r; lift_op(r, a, b, 4, -); return r;
#endif
        }

        v4 V4API sub3(v4 a, v4 b) {
#ifdef USE_SSE
            return (v4) { _mm_sub_ps(a.packed, b.packed) };
#else
            v4 r; lift_op3(r, a, b, -); return r;
#endif
        }

        v4 V4API mul(v4 a, v4 b) {
#ifdef USE_SSE
            return (v4) { _mm_mul_ps(a.packed, b.packed) };
#else
            v4 r; lift_op(r, a, b, 4, *); return r;
#endif
        }

        v4 V4API mul3(v4 a, v4 b) {
#ifdef USE_SSE
            return (v4) { _mm_mul_ps(a.packed, b.packed) };
#else
            v4 r; lift_op3(r, a, b, *); return r;
#endif
        }

        float V4API dot(v4 a, v4 b) {
#ifdef USE_SSE41
            return _mm_cvtss_f32(_mm_dp_ps(a.packed, b.packed, 0xF1));
#elif defined(USE_SSE)
            v4 c = mul(a, b);
            return v4a(c) + v4b(c) + v4c(c) + v4d(c);
#else
            float x = 0.f;
            for (int i = 0; i < 4; ++i)
                x += a.packed[i] * b.packed[i];
            return x;
#endif        
        }

        float V4API dot3(v4 a, v4 b) {
#ifdef USE_SSE41
            return _mm_cvtss_f32(_mm_dp_ps(a.packed, b.packed, 0x71));
#elif defined(USE_SSE)
            v4 c = mul3(a, b);
            return v4a(c) + v4b(c) + v4c(c);
#else
            float x = 0.f;
            for (int i = 0; i < 3; ++i)
                x += a.packed[i] * b.packed[i];
            return x;
#endif 
        }
    }
}

#undef lift_op3
#undef lift_op
#undef V4API

#endif
