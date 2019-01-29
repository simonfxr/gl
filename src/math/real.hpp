#ifndef MATH_HPP
#define MATH_HPP

#include "bl/limits.hpp"
#include "math/mdefs.hpp"

#if defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0 &&               \
  HU_COMP_GNULIKE_P
#    define MATH_IMPORT_ATTRS_F(fn)                                            \
        throw() __asm__(""                                                     \
                        "__" #fn "_finite")
#endif

#define MATH_IMPORT_ATTRS_N(fn) throw()

#ifndef MATH_IMPORT_ATTRS_F
#    define MATH_IMPORT_ATTRS_F(fn) MATH_IMPORT_ATTRS_N(fn)
#endif

#define MATH_IMPORT_ATTRS(fn, suf) PP_CAT(MATH_IMPORT_ATTRS_, suf)(fn)

#define MATH_WRAPPER_TY_1(ty, cnm, nm, suf)                                    \
    extern "C" ty cnm(ty x) MATH_IMPORT_ATTRS(cnm, suf);                       \
    namespace math {                                                           \
    HU_FORCE_INLINE inline ty nm(ty x) noexcept { return ::cnm(x); }           \
    }

#define MATH_WRAPPER_TY_2(ty, cnm, nm, suf)                                    \
    extern "C" ty cnm(ty x, ty y) MATH_IMPORT_ATTRS(cnm, suf);                 \
    namespace math {                                                           \
    HU_FORCE_INLINE inline ty nm(ty x, ty y) noexcept { return ::cnm(x, y); }  \
    }

#define MATH_WRAPPER_NAMED_1(cnm, nm, suf)                                     \
    MATH_WRAPPER_TY_1(float, PP_CAT(cnm, f), nm, suf)                          \
    MATH_WRAPPER_TY_1(double, cnm, nm, suf)

#define MATH_WRAPPER_NAMED_2(cnm, nm, suf)                                     \
    MATH_WRAPPER_TY_2(float, PP_CAT(cnm, f), nm, suf)                          \
    MATH_WRAPPER_TY_2(double, cnm, nm, suf)

#define MATH_WRAPPER_1(nm, suf) MATH_WRAPPER_NAMED_1(nm, nm, suf)

#define MATH_WRAPPER_2(nm, suf) MATH_WRAPPER_NAMED_2(nm, nm, suf)

BEGIN_NO_WARN_DEPRECATED_EX_SPEC

MATH_WRAPPER_NAMED_1(fabs, abs, N)
MATH_WRAPPER_2(fmod, F);

MATH_WRAPPER_1(exp, F)
MATH_WRAPPER_1(exp2, F)
MATH_WRAPPER_1(expm1, N)
MATH_WRAPPER_1(log, F)
MATH_WRAPPER_1(log10, F)
MATH_WRAPPER_1(log2, F)
MATH_WRAPPER_1(log1p, N)

MATH_WRAPPER_2(pow, F)
MATH_WRAPPER_1(sqrt, F)
MATH_WRAPPER_1(cbrt, N)
MATH_WRAPPER_2(hypot, F)

MATH_WRAPPER_1(sin, N)
MATH_WRAPPER_1(cos, N)
MATH_WRAPPER_1(tan, N)
MATH_WRAPPER_1(asin, F)
MATH_WRAPPER_1(acos, F)
MATH_WRAPPER_1(atan, F)
MATH_WRAPPER_2(atan2, F)

MATH_WRAPPER_1(sinh, F)
MATH_WRAPPER_1(cosh, F)
MATH_WRAPPER_1(tanh, F)
MATH_WRAPPER_1(asinh, N)
MATH_WRAPPER_1(acosh, F)
MATH_WRAPPER_1(atanh, F)

MATH_WRAPPER_1(ceil, N)
MATH_WRAPPER_1(floor, N);
MATH_WRAPPER_1(round, N);
MATH_WRAPPER_1(truncate, N);

END_NO_WARN_DEPRECATED_EX_SPEC

namespace math {

#ifdef MATH_REAL_FLOAT
using real = float;
#    define R_FMT "f"
#elif defined(MATH_REAL_DOUBLE)
using real = double;
#    define R_FMT "lf"
#endif

inline constexpr real PI = real(3.1415926535897932384626433832L);

inline constexpr real REAL_MAX = bl::numeric_limits<real>::max();

inline constexpr real REAL_MIN = -REAL_MAX;

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
    x -= floor(x * (1 / (2 * PI))) * (2 * PI);
    x -= PI;
    return x;
}

inline real
wrap(real x, real period)
{
    return fmod(x, period);
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

#define DEF_IABS(t)                                                            \
    HU_FORCE_INLINE                                                            \
    inline constexpr t abs(t x) noexcept { return x < 0 ? -x : x; }

DEF_IABS(int)
DEF_IABS(long)
DEF_IABS(long long)

#undef DEF_IABS

} // namespace math

#endif
