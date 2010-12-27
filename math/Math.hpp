#ifndef _MATH_HPP
#define _MATH_HPP

#include "defs.h"
#include <cmath>

#ifdef USE_SSE
#include <xmmintrin.h>
#endif

namespace Math {

namespace {

const float PI = M_PI;

float sqrt(float x) {
#ifdef USE_SSE
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
#else
    return sqrtf(x);
#endif
}

float recp(float x) {
#ifdef USE_SSE
    return _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(x)));
#else
    return 1 / x;
#endif
}

float rsqrt(float x) {
#ifdef USE_SSE
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
#else
    return recp(sqrt(x));
#endif
}

float wrap(float x, float r) {
    if (unlikely(x < -r || x > r))
        return fmodf(x, r);
    else
        return x;
}

float degToRad(float x) {
    return x * (PI / 180.f);
}

float degToRadWrap(float x) {
    return degToRad(wrap(x, 360.f));
}

float radToDeg(float x) {
    return x * (180.f / PI);
}

float radToDegWrap(float x) {
    return radToDeg(wrap(x, 2 * PI));
}

void sincos(float rad, float *sin_x, float *cos_x) {
    *sin_x = sinf(rad);
    *cos_x = cosf(rad);
}

float tan(float rad) {
    return tanf(rad);
}

float rtan(float rad) {
    return Math::recp(tan(rad));
}

float abs(float x) {
    return fabs(x);
}

float wrapPi(float x) {
    x += PI;
    x -= floor(x * (1 / (2 * PI))) * (2 * PI);
    x -= PI;
    return x;
}

float asin(float x) {
    return asinf(x);
}

float acos(float x) {
    return acosf(x);
}

float atan2(float x, float y) {
    return atan2f(x, y);
}

float distance(float x, float y) {
    return abs(x - y);
}

} // namespace anon

} // namespace Math

#endif
