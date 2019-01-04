#ifndef MATH_HPP
#define MATH_HPP

#include "math/mdefs.hpp"

#include <cmath>
#include <limits>

namespace math {

#ifdef MATH_REAL_FLOAT
using real = float;
#define R_FMT "f"
#elif defined(MATH_REAL_DOUBLE)
using real = double;
#define R_FMT "lf"
#endif

inline constexpr real PI = real(3.1415926535897932384626433832);

inline constexpr real POS_INF = std::numeric_limits<real>::infinity();

inline constexpr real NEG_INF = -POS_INF;

inline real
sqrt(real x)
{
    return std::sqrt(x);
}

inline constexpr real
recip(real x)
{
    return real(1) / x;
}

inline constexpr real
inverse(real x)
{
    return recip(x);
}

inline real
rsqrt(real x)
{
    return recip(sqrt(x));
}

inline real
sin(real rad)
{
    return std::sin(rad);
}

inline real
cos(real rad)
{
    return std::cos(rad);
}

inline real
tan(real rad)
{
    return std::tan(rad);
}

inline real
asin(real x)
{
    return std::asin(x);
}

inline real
acos(real x)
{
    return std::acos(x);
}

inline real
atan(real x)
{
    return std::atan(x);
}

inline real
atan2(real x, real y)
{
    return std::atan2(x, y);
}

inline void
sincos(real rad, real &out_sin, real &out_cos)
{
    out_sin = sin(rad);
    out_cos = cos(rad);
}

inline real
cotan(real rad)
{
    return recip(tan(rad));
}

inline real
exp(real x)
{
    return std::exp(x);
}

inline real
abs(real x)
{
    return std::fabs(x);
}

inline real
length(real x)
{
    return abs(x);
}

inline real
distance(real x, real y)
{
    return length(x - y);
}

inline constexpr real
squared(real x)
{
    return x * x;
}

inline constexpr real
cubed(real x)
{
    return x * x * x;
}

inline constexpr real
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

inline real
pow(real x, real y)
{
    return std::pow(x, y);
}

inline real
floor(real x)
{
    return std::floor(x);
}

inline constexpr int32_t
signum(real x)
{
    return x < real(0) ? -1 : x > real(0) ? +1 : 0;
}

// bool signbit(real x) {
//     return x < 0.f;
// }

inline real
wrapPi(real x)
{
    x += PI;
    x -= std::floor(x * (1 / (2 * PI))) * (2 * PI);
    x -= PI;
    return x;
}

inline real
wrap(real x, real period)
{
    return std::fmod(x, period);
}

inline real
degToRad(real deg)
{
    return deg * (PI / real(180));
}

inline real
radToDeg(real rad)
{
    return rad * (real(180) / PI);
}

inline constexpr real
max(real x, real y)
{
    return x < y ? y : x;
}

constexpr real
min(real x, real y)
{
    return x > y ? y : x;
}

inline constexpr real
clamp(real x, real lo, real hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

inline constexpr real
saturate(real x)
{
    return clamp(x, 0.f, 1.f);
}

inline constexpr real
smoothstep(real lo_edge, real hi_edge, real x)
{
    x = saturate((x - lo_edge) / (hi_edge - lo_edge));
    return x * x * (real(3) - 2 * x);
}

inline constexpr real
mix(real a, real b, real t)
{
    return a + t * (b - a);
}

inline constexpr real operator"" _r(long double x)
{
    return real(x);
}

inline constexpr real operator"" _r(unsigned long long x)
{
    return real(x);
}

} // namespace math

#endif
