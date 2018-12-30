#ifndef VEC3_DEFNS_HPP
#define VEC3_DEFNS_HPP

#include "math/ivec3/type.hpp"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"

MATH_BEGIN_NAMESPACE

constexpr MATH_FUNC vec3_t
vec3(real x, real y, real z) PURE_FUNC;

constexpr MATH_FUNC vec3_t
vec3(real a) PURE_FUNC;

constexpr MATH_FUNC vec3_t
vec3(const ivec3_t &a) PURE_FUNC;

constexpr MATH_FUNC vec3_t
vec3(const vec4_t &a) PURE_FUNC;

constexpr MATH_FUNC vec3_t
vec3(const vec3_t::buffer) PURE_FUNC;

constexpr MATH_FUNC void
load(vec3_t::buffer, const vec3_t &) MUT_FUNC;

constexpr MATH_FUNC vec3_t
operator-(const vec3_t &a) PURE_FUNC;

constexpr MATH_FUNC vec3_t
operator+(const vec3_t &a, const vec3_t b) PURE_FUNC;

constexpr MATH_FUNC vec3_t
operator-(const vec3_t &a, const vec3_t b) PURE_FUNC;

constexpr MATH_FUNC vec3_t operator*(const vec3_t &v, real a) PURE_FUNC;

constexpr MATH_FUNC vec3_t operator*(real a, const vec3_t &v) PURE_FUNC;

constexpr MATH_FUNC vec3_t operator*(const vec3_t &a,
                                     const vec3_t &b) PURE_FUNC;

constexpr MATH_FUNC vec3_t
operator/(const vec3_t &v, real a) PURE_FUNC;

constexpr MATH_FUNC vec3_t
operator/(const vec3_t &v, const vec3_t &b) PURE_FUNC;

constexpr MATH_FUNC vec3_t &
operator+=(vec3_t &v, const vec3_t &a) MUT_FUNC;

constexpr MATH_FUNC vec3_t &
operator-=(vec3_t &v, const vec3_t &a) MUT_FUNC;

constexpr MATH_FUNC vec3_t &
operator*=(vec3_t &v, real a) MUT_FUNC;

constexpr MATH_FUNC vec3_t &
operator*=(vec3_t &v, const vec3_t &b) MUT_FUNC;

constexpr MATH_FUNC vec3_t &
operator/=(vec3_t &v, real a) MUT_FUNC;

constexpr MATH_FUNC vec3_t &
operator/=(vec3_t &v, const vec3_t &b) MUT_FUNC;

constexpr MATH_FUNC bool
operator==(const vec3_t &a, const vec3_t &b) PURE_FUNC;

constexpr MATH_FUNC bool
operator!=(const vec3_t &a, const vec3_t &b) PURE_FUNC;

constexpr MATH_FUNC real
dot(const vec3_t &a, const vec3_t &b) PURE_FUNC;

constexpr MATH_FUNC vec3_t
cross(const vec3_t &a, const vec3_t &b) PURE_FUNC;

MATH_FUNC real
length(const vec3_t &a) PURE_FUNC;

MATH_FUNC real
inverseLength(const vec3_t &a) PURE_FUNC;

constexpr MATH_FUNC real
lengthSq(const vec3_t &a) PURE_FUNC;

MATH_FUNC direction3_t
normalize(const vec3_t &a) PURE_FUNC;

MATH_FUNC real
distance(const vec3_t &a, const vec3_t &b) PURE_FUNC;

MATH_FUNC real
inverseDistance(const vec3_t &a, const vec3_t &b) PURE_FUNC;

constexpr MATH_FUNC real
distanceSq(const vec3_t &a, const vec3_t &b) PURE_FUNC;

constexpr MATH_FUNC vec3_t
reflect(const vec3_t &a, const normal3_t &d) PURE_FUNC;

constexpr MATH_FUNC vec3_t
reflect(const vec3_t &a, const normal3_t &d, real amp) PURE_FUNC;

constexpr MATH_FUNC vec3_t
min(const vec3_t &a, const vec3_t &max) PURE_FUNC;

constexpr MATH_FUNC vec3_t
max(const vec3_t &a, const vec3_t &max) PURE_FUNC;

constexpr MATH_FUNC real
sum(const vec3_t &a) PURE_FUNC;

constexpr MATH_FUNC vec3_t
recip(const vec3_t &a) PURE_FUNC;

constexpr MATH_FUNC vec3_t
abs(const vec3_t &a) PURE_FUNC;

constexpr MATH_FUNC vec3_t
linearInterpolate(const vec3_t &a, const vec3_t &b, real t) PURE_FUNC;

MATH_FUNC direction3_t
directionFromTo(const point3_t &a, const point3_t &b) PURE_FUNC;

MATH_FUNC real
cos(const vec3_t &a, const vec3_t &b) PURE_FUNC;

constexpr MATH_FUNC vec3_t
projectAlong(const vec3_t &a, const vec3_t &x) PURE_FUNC;

constexpr MATH_FUNC bool
equal(const vec3_t &a, const vec3_t &b, real epsi = real(1e-4)) PURE_FUNC;

MATH_END_NAMESPACE

#endif
