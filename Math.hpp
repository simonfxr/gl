#ifndef _MATH_HPP
#define _MATH_HPP

#include "defs.h"
#include <cmath>
#include <xmmintrin.h>

namespace {

    class Math {
    public:

        static const real32 PI;

        static real32 sqrt(real32 x) {
            return sqrtf(x);
        }

        static real32 rsqrt(real32 x) {
            return recp(sqrt(x));
        }

        static real32 recp(real32 x) {
            return unlikely(x == 0.f) ? 0.f : 1 / x;
        }

        static real32 wrap(real32 x, real32 r) {
            if (unlikely(x < -r) || unlikely(x > r))
                return fmodf(x, r);
            else
                return x;
        }

        static real32 degToRad(real32 x) {
            return x * (PI / 180.f);
        }

        static real32 degToRadWrap(real32 x) {
            return degToRad(wrap(x, 360.f));
        }

        static real32 radToDeg(real32 x) {
            return x * (180.f / PI);
        }

        static real32 radToDegWrap(real32 x) {
            return radToDeg(wrap(x, 2 * PI));
        }

    private:
        Math();
        ~Math();
    };

    class FastMath : public Math {
    public:

        static real32 sqrt(real32 x) {
            return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
        }
        
        static real32 recp(real32 x) {
            return _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(x)));
        }
        
        static real32 rsqrt(real32 x) {
            return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
        }

    private:
        FastMath();
        ~FastMath();
    };

    const real32 Math::PI = M_PI;

}
#endif
