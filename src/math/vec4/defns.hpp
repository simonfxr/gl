#ifndef VEC4_DEFNS_HPP
#define VEC4_DEFNS_HPP

#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"

MATH_BEGIN_NAMESPACE

constexpr MATH_FUNC const real *
begin(const vec4_t &) PURE_FUNC;

constexpr MATH_FUNC real *
begin(vec4_t &) MUT_FUNC;

constexpr MATH_FUNC vec4_t
vec4(real x, real y, real z, real w) PURE_FUNC;

constexpr MATH_FUNC vec4_t
vec4(real a) PURE_FUNC;

constexpr MATH_FUNC vec4_t
vec4(const vec3_t &a, real w) PURE_FUNC;

constexpr MATH_FUNC vec4_t
vec4(const real a[4]) PURE_FUNC;

constexpr MATH_FUNC void
load(vec4_t::buffer, const vec4_t &) MUT_FUNC;

constexpr MATH_FUNC vec4_t
operator-(const vec4_t &a) PURE_FUNC;

constexpr MATH_FUNC vec4_t
operator+(const vec4_t &a, const vec4_t b) PURE_FUNC;

constexpr MATH_FUNC vec4_t
operator-(const vec4_t &a, const vec4_t b) PURE_FUNC;

constexpr MATH_FUNC vec4_t operator*(const vec4_t &v, real a) PURE_FUNC;

constexpr MATH_FUNC vec4_t operator*(real a, const vec4_t &v) PURE_FUNC;

constexpr MATH_FUNC vec4_t operator*(const vec4_t &a,
                                     const vec4_t &b) PURE_FUNC;

constexpr MATH_FUNC vec4_t
operator/(const vec4_t &v, real a) PURE_FUNC;

constexpr MATH_FUNC vec4_t &
operator+=(vec4_t &v, const vec4_t &a) MUT_FUNC;

constexpr MATH_FUNC vec4_t &
operator-=(vec4_t &v, const vec4_t &a) MUT_FUNC;

constexpr MATH_FUNC vec4_t &
operator*=(vec4_t &v, real a) MUT_FUNC;

constexpr MATH_FUNC vec4_t &
operator*=(vec4_t &v, const vec4_t &b) MUT_FUNC;

constexpr MATH_FUNC vec4_t &
operator/=(vec4_t &v, real a) MUT_FUNC;

constexpr MATH_FUNC bool
operator==(const vec4_t &a, const vec4_t &b) PURE_FUNC;

constexpr MATH_FUNC bool
operator!=(const vec4_t &a, const vec4_t &b) PURE_FUNC;

constexpr MATH_FUNC real
dot(const vec4_t &a, const vec4_t &b) PURE_FUNC;

MATH_FUNC real
length(const vec4_t &a) PURE_FUNC;

MATH_FUNC real
inverseLength(const vec4_t &a) PURE_FUNC;

constexpr MATH_FUNC real
lengthSq(const vec4_t &a) PURE_FUNC;

MATH_FUNC vec4_t
normalize(const vec4_t &a) PURE_FUNC;

MATH_FUNC real
distance(const vec4_t &a, const vec4_t &b) PURE_FUNC;

MATH_FUNC real
inverseDistance(const vec4_t &a, const vec4_t &b) PURE_FUNC;

constexpr MATH_FUNC real
distanceSq(const vec4_t &a, const vec4_t &b) PURE_FUNC;

constexpr MATH_FUNC vec4_t
min(const vec4_t &a, const vec4_t &max) PURE_FUNC;

constexpr MATH_FUNC vec4_t
max(const vec4_t &a, const vec4_t &max) PURE_FUNC;

constexpr MATH_FUNC real
sum(const vec4_t &a) PURE_FUNC;

MATH_FUNC bool
equal(const vec4_t &a, const vec4_t &b, real epsi = real(1e-4)) PURE_FUNC;

MATH_END_NAMESPACE

#endif
