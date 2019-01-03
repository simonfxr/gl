#ifndef VEC4_DEFNS_HPP
#define VEC4_DEFNS_HPP

#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"

namespace math {

constexpr inline const real *
begin(const vec4_t &);

constexpr inline real *
begin(vec4_t &);

constexpr inline vec4_t
vec4(real x, real y, real z, real w);

constexpr inline vec4_t
vec4(real a);

constexpr inline vec4_t
vec4(const vec3_t &a, real w);

constexpr inline vec4_t
vec4(const real a[4]);

constexpr inline void
load(vec4_t::buffer, const vec4_t &);

constexpr inline vec4_t
operator-(const vec4_t &a);

constexpr inline vec4_t
operator+(const vec4_t &a, const vec4_t b);

constexpr inline vec4_t
operator-(const vec4_t &a, const vec4_t b);

constexpr inline vec4_t operator*(const vec4_t &v, real a);

constexpr inline vec4_t operator*(real a, const vec4_t &v);

constexpr inline vec4_t operator*(const vec4_t &a, const vec4_t &b);

constexpr inline vec4_t
operator/(const vec4_t &v, real a);

constexpr inline vec4_t &
operator+=(vec4_t &v, const vec4_t &a);

constexpr inline vec4_t &
operator-=(vec4_t &v, const vec4_t &a);

constexpr inline vec4_t &
operator*=(vec4_t &v, real a);

constexpr inline vec4_t &
operator*=(vec4_t &v, const vec4_t &b);

constexpr inline vec4_t &
operator/=(vec4_t &v, real a);

constexpr inline bool
operator==(const vec4_t &a, const vec4_t &b);

constexpr inline bool
operator!=(const vec4_t &a, const vec4_t &b);

constexpr inline real
dot(const vec4_t &a, const vec4_t &b);

inline real
length(const vec4_t &a);

inline real
inverseLength(const vec4_t &a);

constexpr inline real
lengthSq(const vec4_t &a);

inline vec4_t
normalize(const vec4_t &a);

inline real
distance(const vec4_t &a, const vec4_t &b);

inline real
inverseDistance(const vec4_t &a, const vec4_t &b);

constexpr inline real
distanceSq(const vec4_t &a, const vec4_t &b);

constexpr inline vec4_t
min(const vec4_t &a, const vec4_t &max);

constexpr inline vec4_t
max(const vec4_t &a, const vec4_t &max);

constexpr inline real
sum(const vec4_t &a);

inline bool
equal(const vec4_t &a, const vec4_t &b, real epsi = real(1e-4));

} // namespace math

#endif
