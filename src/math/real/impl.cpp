#include "math/real/defns.hpp"

#ifdef MATH_REAL_FLOAT
#define MSUF f
#else
#define MSUF
#endif

#ifndef NO_MATH_H
#include <cmath>
#define MATH_CIMPORT(ret, fun, ...)
#define MATH_CFUNC(f) CONCAT(::f, MSUF)
#else
#ifdef SYSTEM_WINDOWS
#define MATH_CIMPORT(ret, fun, ...)                                            \
    extern "C" ret fun(__VA_ARGS__) ATTRS(ATTR_NOTHROW)
#else
#define MATH_CIMPORT(ret, fun, ...)                                            \
    extern "C" ret fun(__VA_ARGS__) ATTRS(ATTR_NOTHROW)
#endif
#define MATH_CFUNC(f) CONCAT(::math::cmath::f, MSUF)
#endif

#define MATH_CIMPORT1(fun) MATH_CIMPORT(real, CONCAT(fun, MSUF), real)
#define MATH_CIMPORT2(fun) MATH_CIMPORT(real, CONCAT(fun, MSUF), real, real)

MATH_BEGIN_NAMESPACE

#ifdef NO_MATH_H

namespace cmath {

MATH_CIMPORT1(sqrt);
MATH_CIMPORT1(sin);
MATH_CIMPORT1(cos);
MATH_CIMPORT1(tan);
MATH_CIMPORT1(asin);
MATH_CIMPORT1(acos);
MATH_CIMPORT1(atan);
MATH_CIMPORT2(atan2);
MATH_CIMPORT2(fmod);
MATH_CIMPORT2(pow);
MATH_CIMPORT1(floor);
MATH_CIMPORT1(exp);
#ifndef SYSTEM_WINDOWS
MATH_CIMPORT1(fabs);
#endif

} // namespace cmath

#endif

using namespace defs;

real
sqrt(real x)
{
    return MATH_CFUNC(sqrt)(x);
}

real
recip(real x)
{
    return real(1) / x;
}

real
inverse(real x)
{
    return recip(x);
}

real
rsqrt(real x)
{
    return recip(sqrt(x));
}

real
sin(real rad)
{
    return MATH_CFUNC(sin)(rad);
}

real
cos(real rad)
{
    return MATH_CFUNC(cos)(rad);
}

real
tan(real rad)
{
    return MATH_CFUNC(tan)(rad);
}

real
asin(real x)
{
    return MATH_CFUNC(asin)(x);
}

real
acos(real x)
{
    return MATH_CFUNC(acos)(x);
}

real
atan(real x)
{
    return MATH_CFUNC(atan)(x);
}

real
atan2(real x, real y)
{
    return MATH_CFUNC(atan2)(x, y);
}

void
sincos(real rad, real &out_sin, real &out_cos)
{
    out_sin = sin(rad);
    out_cos = cos(rad);
}

real
cotan(real rad)
{
    return recip(tan(rad));
}

real
exp(real x)
{
    return MATH_CFUNC(exp)(x);
}

real
abs(real x)
{
#ifdef SYSTEM_WINDOWS
    return x < real(0) ? -x : x;
#else
    return MATH_CFUNC(fabs)(x);
#endif
}

real
length(real x)
{
    return abs(x);
}

real
distance(real x, real y)
{
    return length(x - y);
}

real
squared(real x)
{
    return x * x;
}

real
cubed(real x)
{
    return x * x * x;
}

real
pow(real x, int32 n)
{
    if (n == 0)
        return 1.f;

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

real
pow(real x, real y)
{
    return MATH_CFUNC(pow)(x, y);
}

real
floor(real x)
{
    return MATH_CFUNC(floor)(x);
}

int32
signum(real x)
{
    return x < real(0) ? -1 : x > real(0) ? +1 : 0;
}

// bool signbit(real x) {
//     return x < 0.f;
// }

real
wrapPi(real x)
{
    x += PI;
    x -= MATH_CFUNC(floor)(x * (1 / (2 * PI))) * (2 * PI);
    x -= PI;
    return x;
}

real
wrap(real x, real period)
{
#ifdef MATH_MATH_INLINE
    if (unlikely(x < -period || x > period))
        return MATH_CFUNC(fmod)(x, period);
    else
        return x;
#else
    return MATH_CFUNC(fmod)(x, period);
#endif
}

real
degToRad(real deg)
{
    return deg * (PI / real(180));
}

real
radToDeg(real rad)
{
    return rad * (real(180) / PI);
}

real
max(real x, real y)
{
    return x < y ? y : x;
}

real
min(real x, real y)
{
    return x > y ? y : x;
}

real
saturate(real x)
{
    return clamp(x, 0.f, 1.f);
}

real
clamp(real x, real lo, real hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

real
smoothstep(real lo_edge, real hi_edge, real x)
{
    x = saturate((x - lo_edge) / (hi_edge - lo_edge));
    return x * x * (real(3) - 2 * x);
}

real
mix(real a, real b, real t)
{
    return a + t * (b - a);
}

MATH_END_NAMESPACE

#undef MATH_CIMPORT
#undef MATH_CFUNC
