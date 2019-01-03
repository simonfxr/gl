#ifndef VEC3_DEFNS_HPP
#define VEC3_DEFNS_HPP

#include "math/ivec3/type.hpp"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"

namespace math {

constexpr inline vec3_t
vec3(real x, real y, real z);

constexpr inline vec3_t
vec3(real a);

constexpr inline vec3_t
vec3(const ivec3_t &a);

constexpr inline vec3_t
vec3(const vec4_t &a);

constexpr inline vec3_t
vec3(const vec3_t::buffer);

constexpr inline void
load(vec3_t::buffer, const vec3_t &);

constexpr inline vec3_t
operator-(const vec3_t &a);

constexpr inline vec3_t
operator+(const vec3_t &a, const vec3_t b);

constexpr inline vec3_t
operator-(const vec3_t &a, const vec3_t b);

constexpr inline vec3_t operator*(const vec3_t &v, real a);

constexpr inline vec3_t operator*(real a, const vec3_t &v);

constexpr inline vec3_t operator*(const vec3_t &a, const vec3_t &b);

constexpr inline vec3_t
operator/(const vec3_t &v, real a);

constexpr inline vec3_t
operator/(const vec3_t &v, const vec3_t &b);

constexpr inline vec3_t &
operator+=(vec3_t &v, const vec3_t &a);

constexpr inline vec3_t &
operator-=(vec3_t &v, const vec3_t &a);

constexpr inline vec3_t &
operator*=(vec3_t &v, real a);

constexpr inline vec3_t &
operator*=(vec3_t &v, const vec3_t &b);

constexpr inline vec3_t &
operator/=(vec3_t &v, real a);

constexpr inline vec3_t &
operator/=(vec3_t &v, const vec3_t &b);

constexpr inline bool
operator==(const vec3_t &a, const vec3_t &b);

constexpr inline bool
operator!=(const vec3_t &a, const vec3_t &b);

constexpr inline real
dot(const vec3_t &a, const vec3_t &b);

constexpr inline vec3_t
cross(const vec3_t &a, const vec3_t &b);

inline real
length(const vec3_t &a);

inline real
inverseLength(const vec3_t &a);

constexpr inline real
lengthSq(const vec3_t &a);

inline direction3_t
normalize(const vec3_t &a);

inline real
distance(const vec3_t &a, const vec3_t &b);

inline real
inverseDistance(const vec3_t &a, const vec3_t &b);

constexpr inline real
distanceSq(const vec3_t &a, const vec3_t &b);

constexpr inline vec3_t
reflect(const vec3_t &a, const normal3_t &d);

constexpr inline vec3_t
reflect(const vec3_t &a, const normal3_t &d, real amp);

constexpr inline vec3_t
min(const vec3_t &a, const vec3_t &max);

constexpr inline vec3_t
max(const vec3_t &a, const vec3_t &max);

constexpr inline real
sum(const vec3_t &a);

constexpr inline vec3_t
recip(const vec3_t &a);

inline vec3_t
abs(const vec3_t &a);

constexpr inline vec3_t
linearInterpolate(const vec3_t &a, const vec3_t &b, real t);

inline direction3_t
directionFromTo(const point3_t &a, const point3_t &b);

inline real
cos(const vec3_t &a, const vec3_t &b);

constexpr inline vec3_t
projectAlong(const vec3_t &a, const vec3_t &x);

inline bool
equal(const vec3_t &a, const vec3_t &b, real epsi = real(1e-4));

} // namespace math

#endif
