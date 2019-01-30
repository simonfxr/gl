#ifndef MATH_HPP
#define MATH_HPP

#include "bl/limits.hpp"
#include "math/mdefs.hpp"

#if HU_COMP_GCC_P || HU_COMP_INTEL_P
#    define MATH_FN_CONSTEXPR constexpr
#else
#    define MATH_FN_CONSTEXPR
#endif

#define MATH_FN_C_IMPORT(fn, ret, ...) extern "C" ret fn(__VA_ARGS__) throw();

#if HU_COMP_GNULIKE_P
#    define MATH_FN_BUILTIN_IMPORT(fn, ...)
#    define MATH_FN_BUILTIN_NAME(fn) PP_CAT(__builtin_, fn)
#else
#    define MATH_FN_BUILTIN_IMPORT(fn, ret, ...)                               \
        MATH_FN_C_IMPORT(fn, ret, __VA_ARGS__)
#    define MATH_FN_BUILTIN_NAME(fn) fn
#endif

#define MATH_FN_INLINE HU_FORCE_INLINE inline

#define MATH_WRAP_BUILTIN_TY_1(ret, cret, ty, cnm, nm, suf)                    \
    MATH_FN_BUILTIN_IMPORT(cnm, cret, ty)                                      \
    namespace math {                                                           \
    MATH_FN_INLINE MATH_FN_CONSTEXPR ret nm(ty x) noexcept                     \
    {                                                                          \
        return ret(MATH_FN_BUILTIN_NAME(cnm)(x));                              \
    }                                                                          \
    }

#define MATH_WRAP_C_TY_1(ret, cret, ty, cnm, nm, suf)                          \
    MATH_FN_C_IMPORT(cnm, cret, ty)                                            \
    namespace math {                                                           \
    MATH_FN_INLINE ret nm(ty x) noexcept { return ret(::cnm(x)); }             \
    }

#define MATH_WRAP_BUILTIN_TY_2(ret, cret, ty, cnm, nm, suf)                    \
    MATH_FN_BUILTIN_IMPORT(cnm, cret, ty, ty)                                  \
    namespace math {                                                           \
    MATH_FN_INLINE MATH_FN_CONSTEXPR ret nm(ty x, ty y) noexcept               \
    {                                                                          \
        return ret(MATH_FN_BUILTIN_NAME(cnm)(x, y));                           \
    }                                                                          \
    }

#define MATH_WRAP_C_TY_2(ret, cret, ty, cnm, nm, suf)                          \
    MATH_FN_C_IMPORT(cnm, cret, ty, ty)                                        \
    namespace math {                                                           \
    MATH_FN_INLINE ret nm(ty x, ty y) noexcept { return ret(::cnm(x, y)); }    \
    }

#define MATH_WRAP_BUILTIN_NAMED_RET_1(cnm, nm, ret, cret, suf)                 \
    MATH_WRAP_BUILTIN_TY_1(ret, cret, float, PP_CAT(cnm, f), nm, suf)          \
    MATH_WRAP_BUILTIN_TY_1(ret, cret, double, cnm, nm, suf)

#define MATH_WRAP_C_NAMED_RET_1(cnm, nm, ret, cret, suf)                       \
    MATH_WRAP_C_TY_1(ret, cret, float, PP_CAT(cnm, f), nm, suf)                \
    MATH_WRAP_C_TY_1(ret, cret, double, cnm, nm, suf)

#define MATH_WRAP_BUILTIN_NAMED_1(cnm, nm, suf)                                \
    MATH_WRAP_BUILTIN_TY_1(float, float, float, PP_CAT(cnm, f), nm, suf)       \
    MATH_WRAP_BUILTIN_TY_1(double, double, double, cnm, nm, suf)

#define MATH_WRAP_BUILTIN_NAMED_RET_2(cnm, nm, ret, cret, suf)                 \
    MATH_WRAP_BUILTIN_TY_2(ret, cret, float, PP_CAT(cnm, f), nm, suf)          \
    MATH_WRAP_BUILTIN_TY_2(ret, cret, double, cnm, nm, suf)

#define MATH_WRAP_BUILTIN_NAMED_2(cnm, nm, suf)                                \
    MATH_WRAP_BUILTIN_TY_2(float, float, float, PP_CAT(cnm, f), nm, suf)       \
    MATH_WRAP_BUILTIN_TY_2(double, double, double, cnm, nm, suf)

#define MATH_WRAP_BUILTIN_1(cnm, nm, suf)                                      \
    MATH_WRAP_BUILTIN_NAMED_1(cnm, nm, suf)

#define MATH_WRAP_C_1(nm, suf) MATH_WRAP_C_NAMED_1(nm, nm, suf)

#define MATH_WRAP_1(nm, suf) MATH_WRAP_BUILTIN_NAMED_1(nm, nm, suf)

#define MATH_WRAP_RET_1(nm, ret, cret, suf)                                    \
    MATH_WRAP_BUILTIN_NAMED_RET_1(nm, nm, ret, cret, suf)

#define MATH_WRAP_BUILTIN_2(cnm, nm, suf)                                      \
    MATH_WRAP_BUILTIN_NAMED_2(cnm, nm, suf)

#define MATH_WRAP_C_2(nm, suf) MATH_WRAP_C_NAMED_2(nm, nm, suf)

#define MATH_WRAP_2(nm, suf) MATH_WRAP_BUILTIN_NAMED_2(nm, nm, suf)

#if HU_COMP_GNULIKE_P
#    define MATH_WRAP_CLASSIFY(nm)                                             \
        MATH_WRAP_BUILTIN_TY_1(bool, int, float, nm, nm, N)                    \
        MATH_WRAP_BUILTIN_TY_1(bool, int, double, nm, nm, N)
#else
#    define MATH_WRAP_CLASSIFY(nm) MATH_WRAP_C_NAMED_RET_1(nm, nm, int, bool, N)
#endif

BEGIN_NO_WARN_DEPRECATED_EX_SPEC

MATH_WRAP_BUILTIN_NAMED_1(fabs, abs, N)
MATH_WRAP_2(fmod, F)

MATH_WRAP_1(exp, F)
MATH_WRAP_1(exp2, F)
MATH_WRAP_1(expm1, N)

MATH_WRAP_1(log, F)
MATH_WRAP_1(log10, F)
MATH_WRAP_1(log1p, N)
MATH_WRAP_1(log2, F)

MATH_WRAP_2(pow, F)
MATH_WRAP_1(sqrt, F)
MATH_WRAP_1(cbrt, N)
MATH_WRAP_2(hypot, F)

MATH_WRAP_1(sin, N)
MATH_WRAP_1(cos, N)
MATH_WRAP_1(tan, N)
MATH_WRAP_1(asin, F)
MATH_WRAP_1(acos, F)
MATH_WRAP_1(atan, F)
MATH_WRAP_2(atan2, F)

MATH_WRAP_1(sinh, F)
MATH_WRAP_1(cosh, F)
MATH_WRAP_1(tanh, F)
MATH_WRAP_1(asinh, N)
MATH_WRAP_1(acosh, F)
MATH_WRAP_1(atanh, F)

MATH_WRAP_1(erf, F)
MATH_WRAP_1(erfc, F)
MATH_WRAP_1(lgamma, F)
MATH_WRAP_1(tgamma, F)

MATH_WRAP_1(ceil, N)
MATH_WRAP_1(floor, N)
MATH_WRAP_1(round, N)
MATH_WRAP_1(trunc, N)

MATH_WRAP_2(nextafter, N)
MATH_WRAP_2(nexttoward, N)

#if HU_COMP_GNULIKE_P
MATH_WRAP_CLASSIFY(isfinite)
#else
MATH_WRAP_C_NAMED_RET_1(finite, isfinite, int, bool, N)
#endif

MATH_WRAP_CLASSIFY(isnan)
MATH_WRAP_CLASSIFY(isinf)

#if HU_COMP_GNULIKE_P
MATH_WRAP_CLASSIFY(isnormal)
#endif

MATH_WRAP_2(copysign, N);

MATH_WRAP_BUILTIN_NAMED_RET_1(signbit, signbit, bool, int, N)

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

#if !HU_COMP_GNULIKE_P
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
#endif

inline constexpr int32_t
signum(real x)
{
    return x < real(0) ? -1 : x > real(0) ? +1 : 0;
}

inline MATH_FN_CONSTEXPR real
fract(real x) noexcept
{
    return x - floor(x);
}

inline MATH_FN_CONSTEXPR real
wrapPi(real x) noexcept
{
    return fract((x - PI) / (2 * PI)) * (2 * PI) + PI;
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
