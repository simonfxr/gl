#include "math/real/defns.hpp"
#include <cmath>

MATH_BEGIN_NAMESPACE

using namespace defs;

real sqrt(real x) { 
    return sqrtf(x);
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
    return sinf(rad);
}

real cos(real rad) {
    return cosf(rad);
}

real tan(real rad) {
    return tanf(rad);
}

real asin(real x) {
    return asinf(x);
}

real acos(real x) {
    return acosf(x);
}

real atan(real x) {
    return atanf(x);
}

real atan2(real x, real y) {
    return atan2f(x, y);
}

void sincos(real rad, real& out_sin, real& out_cos) {
    out_sin = sin(rad);
    out_cos = cos(rad);
}

real cotan(real rad) {
    return recip(tan(rad));
}

real abs(real x) {
    return fabsf(x);
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
    return powf(x, y);
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
    x -= floorf(x * (1 / (2 * PI))) * (2 * PI);
    x -= PI;
    return x;
}

real wrap(real x, real period) {
#ifdef MATH_MATH_INLINE
    if (unlikely(x < -period || x > period))
        return fmodf(x, period);
    else
        return x;
#else
    return fmodf(x, period);
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
    return x*x*(3 - 2*x);
}

real mix(real a, real b, real t) {
    return a + t * (b - a);
}

MATH_END_NAMESPACE
