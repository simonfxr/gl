#include "math/real/defns.hpp"

#include <cmath>

MATH_BEGIN_NAMESPACE

using namespace defs;

real
sqrt(real x)
{
    return std::sqrt(x);
}

constexpr real
recip(real x)
{
    return real(1) / x;
}

constexpr real
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
    return std::sin(rad);
}

real
cos(real rad)
{
    return std::cos(rad);
}

real
tan(real rad)
{
    return std::tan(rad);
}

real
asin(real x)
{
    return std::asin(x);
}

real
acos(real x)
{
    return std::acos(x);
}

real
atan(real x)
{
    return std::atan(x);
}

real
atan2(real x, real y)
{
    return std::atan2(x, y);
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
    return std::exp(x);
}

constexpr real
abs(real x)
{
    return std::fabs(x);
}

constexpr real
length(real x)
{
    return abs(x);
}

constexpr real
distance(real x, real y)
{
    return length(x - y);
}

constexpr real
squared(real x)
{
    return x * x;
}

constexpr real
cubed(real x)
{
    return x * x * x;
}

real
pow(real x, int32_t n)
{
    if (n == 0)
        return 1.f;

    uint32_t k = n < 0 ? uint32_t(-n) : uint32_t(n);
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
    return std::pow(x, y);
}

real
floor(real x)
{
    return std::floor(x);
}

constexpr int32_t
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
    x -= std::floor(x * (1 / (2 * PI))) * (2 * PI);
    x -= PI;
    return x;
}

real
wrap(real x, real period)
{
    return std::fmod(x, period);
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

constexpr real
max(real x, real y)
{
    return x < y ? y : x;
}

constexpr real
min(real x, real y)
{
    return x > y ? y : x;
}

constexpr real
saturate(real x)
{
    return clamp(x, 0.f, 1.f);
}

constexpr real
clamp(real x, real lo, real hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

constexpr real
smoothstep(real lo_edge, real hi_edge, real x)
{
    x = saturate((x - lo_edge) / (hi_edge - lo_edge));
    return x * x * (real(3) - 2 * x);
}

constexpr real
mix(real a, real b, real t)
{
    return a + t * (b - a);
}

MATH_END_NAMESPACE

#undef MATH_CIMPORT
#undef MATH_CFUNC
