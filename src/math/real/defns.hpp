#ifndef MATH_REAL_DEFNS_HPP
#define MATH_REAL_DEFNS_HPP

#include "math/real/type.hpp"

#include <limits>

namespace math {

inline constexpr real PI = 3.1415926f;

inline constexpr real POS_INF = std::numeric_limits<real>::infinity();

inline constexpr real NEG_INF = -POS_INF;

inline real
sqrt(real x);

constexpr inline real
recip(real x);

constexpr inline real
inverse(real x);

inline real
rsqrt(real x);

inline real
sin(real rad);

inline real
cos(real rad);

inline real
tan(real rad);

inline real
asin(real x);

inline real
acos(real x);

inline real
atan(real x);

inline real
atan2(real x, real y);

inline void
sincos(real rad, real &out_sin, real &out_cos);

inline real
cotan(real rad); // 1/tan

inline real
exp(real x);

inline real
abs(real x);

inline real
length(real x);

inline real
distance(real x, real y);

constexpr inline real
squared(real x);

constexpr inline real
cubed(real x);

inline real
pow(real x, int32_t n);

inline real
pow(real x, real y);

inline real
floor(real x);

constexpr inline int32_t
signum(real x);

// inline bool signbit(real x) ;

inline real
wrapPi(real x);

inline real
wrap(real x, real period);

inline real
degToRad(real deg);

inline real
radToDeg(real rad);

constexpr inline real
max(real x, real y);

constexpr inline real
min(real x, real y);

constexpr inline real
saturate(real x);

constexpr inline real
clamp(real x, real lo, real hi);

constexpr inline real
smoothstep(real lo_edge, real hi_edge, real x);

constexpr inline real
mix(real a, real b, real t);

} // namespace math

#endif
