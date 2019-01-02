#ifndef MATH_REAL_DEFNS_HPP
#define MATH_REAL_DEFNS_HPP

#include "math/real/type.hpp"

#include <limits>

MATH_BEGIN_NAMESPACE

inline constexpr real PI = 3.1415926f;

inline constexpr real POS_INF = std::numeric_limits<real>::infinity();

inline constexpr real NEG_INF = -POS_INF;

MATH_FUNC real
sqrt(real x) PURE_FUNC;

constexpr MATH_FUNC real
recip(real x) PURE_FUNC;

constexpr MATH_FUNC real
inverse(real x) PURE_FUNC;

MATH_FUNC real
rsqrt(real x) PURE_FUNC;

MATH_FUNC real
sin(real rad) PURE_FUNC;

MATH_FUNC real
cos(real rad) PURE_FUNC;

MATH_FUNC real
tan(real rad) PURE_FUNC;

MATH_FUNC real
asin(real x) PURE_FUNC;

MATH_FUNC real
acos(real x) PURE_FUNC;

MATH_FUNC real
atan(real x) PURE_FUNC;

MATH_FUNC real
atan2(real x, real y) PURE_FUNC;

MATH_FUNC void
sincos(real rad, real &out_sin, real &out_cos) PURE_FUNC;

MATH_FUNC real
cotan(real rad) PURE_FUNC; // 1/tan

MATH_FUNC real
exp(real x) PURE_FUNC;

constexpr MATH_FUNC real
abs(real x) PURE_FUNC;

constexpr MATH_FUNC real
length(real x) PURE_FUNC;

constexpr MATH_FUNC real
distance(real x, real y) PURE_FUNC;

constexpr MATH_FUNC real
squared(real x) PURE_FUNC;

constexpr MATH_FUNC real
cubed(real x) PURE_FUNC;

MATH_FUNC real
pow(real x, int32_t n) PURE_FUNC;

MATH_FUNC real
pow(real x, real y) PURE_FUNC;

MATH_FUNC real
floor(real x) PURE_FUNC;

constexpr MATH_FUNC int32_t
signum(real x) PURE_FUNC;

// MATH_FUNC bool signbit(real x) PURE_FUNC;

MATH_FUNC real
wrapPi(real x) PURE_FUNC;

MATH_FUNC real
wrap(real x, real period) PURE_FUNC;

MATH_FUNC real
degToRad(real deg) PURE_FUNC;

MATH_FUNC real
radToDeg(real rad) PURE_FUNC;

constexpr MATH_FUNC real
max(real x, real y) PURE_FUNC;

constexpr MATH_FUNC real
min(real x, real y) PURE_FUNC;

constexpr MATH_FUNC real
saturate(real x) PURE_FUNC;

constexpr MATH_FUNC real
clamp(real x, real lo, real hi) PURE_FUNC;

constexpr MATH_FUNC real
smoothstep(real lo_edge, real hi_edge, real x) PURE_FUNC;

constexpr MATH_FUNC real
mix(real a, real b, real t) PURE_FUNC;

MATH_END_NAMESPACE

#endif
