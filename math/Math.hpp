#ifndef _MATH_HPP
#define _MATH_HPP

#include "defs.h"
#include <cmath>
#include <xmmintrin.h>

#define Math MathGeneric

namespace {

    class MathGeneric {
    public:

        static const float PI;

        static float sqrt(float x) {
            return sqrtf(x);
        }

        static float rsqrt(float x) {
            return recp(sqrt(x));
        }

        static float recp(float x) {
            return 1 / x;
        }

        static float wrap(float x, float r) {
            if (unlikely(x < -r || x > r))
                return fmodf(x, r);
            else
                return x;
        }

        static float degToRad(float x) {
            return x * (PI / 180.f);
        }

        static float degToRadWrap(float x) {
            return degToRad(wrap(x, 360.f));
        }

        static float radToDeg(float x) {
            return x * (180.f / PI);
        }

        static float radToDegWrap(float x) {
            return radToDeg(wrap(x, 2 * PI));
        }

        static void sincos(float rad, float *sin_x, float *cos_x) {
            *sin_x = sinf(rad);
            *cos_x = cosf(rad);
        }

        static float tan(float rad) {
            return tanf(rad);
        }

        static float rtan(float rad) {
            return Math::recp(tan(rad));
        }

        static float abs(float x) {
            return fabs(x);
        }

        static float wrapPi(float x) {
            x += PI;
            x -= floor(x * (1 / (2 * PI))) * (2 * PI);
            x -= PI;
            return x;
        }

        static float asin(float x) {
            return asinf(x);
        }

        static float acos(float x) {
            return acosf(x);
        }

        static float atan2(float x, float y) {
            return atan2f(x, y);
        }

    private:
        Math();
        ~Math();
    };

    class FastMath : public MathGeneric {
    public:

        static float sqrt(float x) {
            return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
        }
        
        static float recp(float x) {
            return _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(x)));
        }
        
        static float rsqrt(float x) {
            return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
        }

    private:
        FastMath();
        ~FastMath();
    };

    const float MathGeneric::PI = M_PI;
}
#endif
