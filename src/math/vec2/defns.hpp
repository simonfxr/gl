#ifndef MATH_VEC2_DEFNS_HPP
#define MATH_VEC2_DEFNS_HPP

#include "math/mdefs.hpp"
#include "math/vec2/type.hpp"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"

namespace math {

constexpr inline vec2_t
vec2(real x, real y);

constexpr inline vec2_t
vec2(real a);

constexpr inline vec2_t
vec2(const vec3_t &a);

constexpr inline vec2_t
vec2(const vec4_t &a);

constexpr inline vec2_t
vec2(const real a[2]);

constexpr inline void
load(vec2_t::buffer, const vec2_t &);

constexpr inline vec2_t
operator-(const vec2_t &a);

constexpr inline vec2_t
operator+(const vec2_t &a, const vec2_t b);

constexpr inline vec2_t
operator-(const vec2_t &a, const vec2_t b);

constexpr inline vec2_t operator*(const vec2_t &v, real a);

constexpr inline vec2_t operator*(real a, const vec2_t &v);

constexpr inline vec2_t operator*(const vec2_t &a, const vec2_t &b);

constexpr inline vec2_t
operator/(const vec2_t &v, real a);

constexpr inline vec2_t
operator/(const vec2_t &v, const vec2_t &a);

constexpr inline vec2_t &
operator+=(vec2_t &v, const vec2_t &a);

constexpr inline vec2_t &
operator-=(vec2_t &v, const vec2_t &a);

constexpr inline vec2_t &
operator*=(vec2_t &v, real a);

constexpr inline vec2_t &
operator*=(vec2_t &v, const vec2_t &b);

constexpr inline vec2_t &
operator/=(vec2_t &v, real a);

constexpr inline bool
operator==(const vec2_t &a, const vec2_t &b);

constexpr inline bool
operator!=(const vec2_t &a, const vec2_t &b);

constexpr inline real
dot(const vec2_t &a, const vec2_t &b);

inline real
length(const vec2_t &a);

inline real
inverseLength(const vec2_t &a);

constexpr inline real
lengthSq(const vec2_t &a);

inline direction2_t
normalize(const vec2_t &a);

inline real
distance(const vec2_t &a, const vec2_t &b);

inline real
inverseDistance(const vec2_t &a, const vec2_t &b);

constexpr inline real
distanceSq(const vec2_t &a, const vec2_t &b);

constexpr inline vec2_t
reflect(const vec2_t &a, const normal2_t &d);

constexpr inline vec2_t
reflect(const vec2_t &a, const normal2_t &d, real amp);

constexpr inline vec2_t
min(const vec2_t &a, const vec2_t &max);

constexpr inline vec2_t
max(const vec2_t &a, const vec2_t &max);

constexpr inline real
sum(const vec2_t &a);

constexpr inline vec2_t
recip(const vec2_t &a);

constexpr inline vec2_t
linearInterpolate(const vec2_t &a, const vec2_t &b, real t);

inline direction2_t
directionFromTo(const point2_t &a, const point2_t &b);

inline real
cos(const vec2_t &a, const vec2_t &b);

constexpr inline vec2_t
projectAlong(const vec2_t &a, const vec2_t &x);

inline bool
equal(const vec2_t &a, const vec2_t &b, real epsi = 1e-4f);
} // namespace math

#endif
