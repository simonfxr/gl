#include "math/real/defns.hpp"

#ifndef NO_MATH_H
#  include <cmath>
#  define MATH_CIMPORT(...)
#  define MATH_CFUNC(f) ::f
#else
#  ifdef SYSTEM_WINDOWS
#    define MATH_CIMPORT(...) extern "C" __VA_ARGS__ ATTRS(ATTR_NOTHROW)
#  else
#    define MATH_CIMPORT(...) extern "C" __VA_ARGS__ ATTRS(ATTR_NOTHROW)
#  endif
#  define MATH_CFUNC(f) ::math::cmath::f
#endif

MATH_BEGIN_NAMESPACE

#ifdef NO_MATH_H

namespace cmath {

MATH_CIMPORT(float sqrtf(float));
MATH_CIMPORT(float sinf(float));
MATH_CIMPORT(float cosf(float));
MATH_CIMPORT(float tanf(float));
MATH_CIMPORT(float asinf(float));
MATH_CIMPORT(float acosf(float));
MATH_CIMPORT(float atanf(float));
MATH_CIMPORT(float atan2f(float, float));
MATH_CIMPORT(float fmodf(float, float));
#ifndef SYSTEM_WINDOWS
  MATH_CIMPORT(float fabsf(float));
#endif
MATH_CIMPORT(float powf(float, float));
MATH_CIMPORT(float floorf(float));

} // namespace cmath

#endif

using namespace defs;

real sqrt(real x) {
    return MATH_CFUNC(sqrtf)(x);
}

real recip(real x) {
    return 1.f / x;
}

real inverse(real x) {
    return recip(x);
}

real rsqrt(real x) {
    return recip(sqrt(x));
}

real sin(real rad) {
    return MATH_CFUNC(sinf)(rad);
}

real cos(real rad) {
    return MATH_CFUNC(cosf)(rad);
}

real tan(real rad) {
    return MATH_CFUNC(tanf)(rad);
}

real asin(real x) {
    return MATH_CFUNC(asinf)(x);
}

real acos(real x) {
    return MATH_CFUNC(acosf)(x);
}

real atan(real x) {
    return MATH_CFUNC(atanf)(x);
}

real atan2(real x, real y) {
    return MATH_CFUNC(atan2f)(x, y);
}

void sincos(real rad, real& out_sin, real& out_cos) {
    out_sin = sin(rad);
    out_cos = cos(rad);
}

real cotan(real rad) {
    return recip(tan(rad));
}

real abs(real x) {
#ifdef SYSTEM_WINDOWS
    return x < real(0) ? - x : x;
#else
    return MATH_CFUNC(fabsf)(x);
#endif
}

real length(real x) {
    return abs(x);
}

real distance(real x, real y) {
    return length(x - y);
}

real squared(real x) {
    return x * x;
}

real cubed(real x) {
    return x * x * x;
}

real pow(real x, int32 n) {
    if (n == 0) return 1.f;
    
    uint32 k = n < 0 ? uint32(-n) : uint32(n);
    real a = 1.f;
    real p = x;

    while (k > 1) {
        if (k % 2 == 1)
            a *= x;
        k /= 2;
        p *= p;
    }

    real r = a * p;
    return n < 0 ? recip(r) : r;
}

real pow(real x, real y) {
    return MATH_CFUNC(powf)(x, y);
}

real floor(real x) {
    return MATH_CFUNC(floorf)(x);
}

int32 signum(real x) {
    return x < 0.f ? -1 :
           x > 0.f ? +1 :
                      0;
}

// bool signbit(real x) {
//     return x < 0.f;
// }

real wrapPi(real x) {
    x += PI;
    x -= MATH_CFUNC(floorf)(x * (1 / (2 * PI))) * (2 * PI);
    x -= PI;
    return x;
}

real wrap(real x, real period) {
#ifdef MATH_MATH_INLINE
    if (unlikely(x < -period || x > period))
        return MATH_CFUNC(fmodf)(x, period);
    else
        return x;
#else
    return MATH_CFUNC(fmodf)(x, period);
#endif
}

real degToRad(real deg) {
    return deg * (PI / 180.f);
}

real radToDeg(real rad) {
    return rad * (180.f / PI);
}

real max(real x, real y) {
    return x < y ? y : x;
}

real min(real x, real y) {
    return x > y ? y : x;
}

real saturate(real x) {
    return clamp(x, 0.f, 1.f);
}

real clamp(real x, real lo, real hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

real smoothstep(real lo_edge, real hi_edge, real x) {
    x = saturate((x - lo_edge)/(hi_edge - lo_edge)); 
    return x*x*(real(3) - 2*x);
}

real mix(real a, real b, real t) {
    return a + t * (b - a);
}

MATH_END_NAMESPACE

#undef MATH_CIMPORT
#undef MATH_CFUNC
