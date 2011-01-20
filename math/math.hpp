#ifndef _MATH_HPP
#define _MATH_HPP

#include "defs.h"
#include <cmath>

#ifdef USE_SSE
#include <xmmintrin.h>
#endif

namespace math {

namespace {

const float PI = M_PI;

float LOCAL sqrt(float x) {
#ifdef USE_SSE
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
#else
    return sqrtf(x);
#endif
}

float LOCAL recp(float x) {
#ifdef USE_SSE
    return _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(x)));
#else
    return 1 / x;
#endif
}

float LOCAL rsqrt(float x) {
#ifdef USE_SSE
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
#else
    return recp(sqrt(x));
#endif
}

float LOCAL wrap(float x, float r) {
    if (unlikely(x < -r || x > r))
        return fmodf(x, r);
    else
        return x;
}

float LOCAL degToRad(float x) {
    return x * (PI / 180.f);
}

float LOCAL degToRadWrap(float x) {
    return degToRad(wrap(x, 360.f));
}

float LOCAL radToDeg(float x) {
    return x * (180.f / PI);
}

float LOCAL radToDegWrap(float x) {
    return radToDeg(wrap(x, 2 * PI));
}

void LOCAL sincos(float rad, float *sin_x, float *cos_x) {
    *sin_x = sinf(rad);
    *cos_x = cosf(rad);
}

float LOCAL tan(float rad) {
    return tanf(rad);
}

float LOCAL rtan(float rad) {
    return recp(tan(rad));
}

float LOCAL abs(float x) {
    return fabs(x);
}

float LOCAL wrapPi(float x) {
    x += PI;
    x -= floor(x * (1 / (2 * PI))) * (2 * PI);
    x -= PI;
    return x;
}

float LOCAL asin(float x) {
    return asinf(x);
}

float LOCAL acos(float x) {
    return acosf(x);
}

float LOCAL atan2(float x, float y) {
    return atan2f(x, y);
}

float LOCAL distance(float x, float y) {
    return abs(x - y);
}

float LOCAL squared(float x) {
    return x * x;
}

float LOCAL cubed(float x) {
    return x * x * x;
}

int32 LOCAL signum(float x) {
    if (x < 0)
        return -1;
    else if (x > 0)
        return +1;
    else
        return 0;
}

} // namespace anon

} // namespace Math

#endif
