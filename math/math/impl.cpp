#if defined(MATH_MATH_INLINE) || !defined(MATH_INLINE)

#include "math/math/defns.hpp"
#include <cmath>

MATH_BEGIN_NAMESPACE

float sqrt(float x) { 
    return sqrtf(x);
}

float recip(float x) {
    return 1.f / x;
}

float inverse(float x) {
    return recip(x);
}

float rsqrt(float x) {
    return recip(sqrt(x));
}

float sin(float rad) {
    return sinf(rad);
}

float cos(float rad) {
    return cosf(rad);
}

float tan(float rad) {
    return tanf(rad);
}

float asin(float x) {
    return asinf(x);
}

float acos(float x) {
    return acosf(x);
}

float atan(float x) {
    return atanf(x);
}

float atan2(float x, float y) {
    return atan2f(x, y);
}

void sincos(float rad, float& out_sin, float& out_cos) {
    out_sin = sin(rad);
    out_cos = cos(rad);
}

float cotan(float rad) {
    return recip(tan(rad));
}

float abs(float x) {
    return fabsf(x);
}

float length(float x) {
    return abs(x);
}

float distance(float x, float y) {
    return length(x - y);
}

float squared(float x) {
    return x * x;
}

float cubed(float x) {
    return x * x * x;
}

float pow(float x, int32 n) {
    if (n == 0) return 1.f;
    
    uint32 k = n < 0 ? -n : n;
    float a = 1.f;
    float p = x;

    while (k > 1) {
        if (k % 2 == 1)
            a *= x;
        k /= 2;
        p *= p;
    }

    float r = a * p;
    return n < 0 ? recip(r) : r;
}

float pow(float x, float y) {
    return powf(x, y);
}


int32 signum(float x) {
    return x < 0.f ? -1 :
           x > 0.f ? +1 :
                      0;
}

// bool signbit(float x) {
//     return x < 0.f;
// }

float wrapPi(float x) {
    x += PI;
    x -= floor(x * (1 / (2 * PI))) * (2 * PI);
    x -= PI;
    return x;
}

float wrap(float x, float period) {
#ifdef MATH_MATH_INLINE
    if (unlikely(x < -period || x > period))
        return fmodf(x, period);
    else
        return x;
#else
    return fmodf(x, period);
#endif
}

float degToRad(float deg) {
    return deg * (PI / 180.f);
}

float radToDeg(float rad) {
    return rad * (180.f / PI);
}

float max(float x, float y) {
    return x < y ? y : x;
}

float min(float x, float y) {
    return x > y ? y : x;
}

float saturate(float x) {
    return clamp(x, 0.f, 1.f);
}

float clamp(float x, float lo, float hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

float smoothstep(float lo_edge, float hi_edge, float x) {
    x = saturate((x - lo_edge)/(hi_edge - lo_edge)); 
    return x*x*(3 - 2*x);
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

MATH_END_NAMESPACE

#endif
