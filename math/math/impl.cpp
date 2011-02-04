#if defined(MATH_MATH_INLINE) || !defined(MATH_INLINE)

#include "math/math/defns.hpp"
#include <cmath>


MATH_BEGIN_NAMESPACE

float sqrt(float x) { 
    return sqrtf(x);
}

float recp(float x) {
    return 1.f / x;
}

float rsqrt(float x) {
    return recp(sqrt(x));
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

float rtan(float rad) {
    return recp(tan(rad));
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

int32 signum(float x) {
    return x < 0.f ? -1 :
           x > 0.f ? +1 :
                      0;
}

int32 signbit(float x) {
    return (x < 0.f) ? 1 : 0;
}

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

MATH_END_NAMESPACE

#endif
