#ifndef MATH_IVEC3_DEFNS_HPP
#define MATH_IVEC3_DEFNS_HPP

#include "math/ivec3/type.hpp"
#include "math/vec3/type.hpp"

MATH_BEGIN_NAMESPACE

constexpr MATH_FUNC ivec3_t
ivec3(defs::int32 x, defs::int32 y, defs::int32 z) PURE_FUNC;

constexpr MATH_FUNC ivec3_t
ivec3(defs::int32 a) PURE_FUNC;

constexpr MATH_FUNC ivec3_t
ivec3(const vec3_t &a) PURE_FUNC;

constexpr MATH_FUNC ivec3_t
ivec3(const defs::int32 a[3]) PURE_FUNC;

constexpr MATH_FUNC ivec3_t
operator-(const ivec3_t &a) PURE_FUNC;

constexpr MATH_FUNC ivec3_t
operator+(const ivec3_t &a, const ivec3_t b) PURE_FUNC;

constexpr MATH_FUNC ivec3_t
operator-(const ivec3_t &a, const ivec3_t b) PURE_FUNC;

constexpr MATH_FUNC ivec3_t operator*(const ivec3_t &v,
                                      defs::int32 a) PURE_FUNC;

constexpr MATH_FUNC ivec3_t operator*(defs::int32 a,
                                      const ivec3_t &v) PURE_FUNC;

constexpr MATH_FUNC ivec3_t operator*(const ivec3_t &a,
                                      const ivec3_t &b) PURE_FUNC;

constexpr MATH_FUNC ivec3_t
operator/(const ivec3_t &v, defs::int32 a) PURE_FUNC;

constexpr MATH_FUNC ivec3_t
operator/(const ivec3_t &v, const ivec3_t &b) PURE_FUNC;

constexpr MATH_FUNC ivec3_t &
operator+=(ivec3_t &v, const ivec3_t &a) MUT_FUNC;

constexpr MATH_FUNC ivec3_t &
operator-=(ivec3_t &v, const ivec3_t &a) MUT_FUNC;

constexpr MATH_FUNC ivec3_t &
operator*=(ivec3_t &v, defs::int32 a) MUT_FUNC;

constexpr MATH_FUNC ivec3_t &
operator*=(ivec3_t &v, const ivec3_t &b) MUT_FUNC;

constexpr MATH_FUNC ivec3_t &
operator/=(ivec3_t &v, defs::int32 a) MUT_FUNC;

constexpr MATH_FUNC ivec3_t &
operator/=(ivec3_t &v, const ivec3_t &b) MUT_FUNC;

constexpr MATH_FUNC bool
operator==(const ivec3_t &a, const ivec3_t &b) PURE_FUNC;

constexpr MATH_FUNC bool
operator!=(const ivec3_t &a, const ivec3_t &b) PURE_FUNC;

constexpr MATH_FUNC defs::int32
dot(const ivec3_t &a, const ivec3_t &b) PURE_FUNC;

constexpr MATH_FUNC ivec3_t
cross(const ivec3_t &a, const ivec3_t &b) PURE_FUNC;

MATH_FUNC real
length(const ivec3_t &a) PURE_FUNC;

MATH_FUNC real
inverseLength(const ivec3_t &a) PURE_FUNC;

constexpr MATH_FUNC defs::int32
lengthSq(const ivec3_t &a) PURE_FUNC;

MATH_FUNC real
distance(const ivec3_t &a, const ivec3_t &b) PURE_FUNC;

MATH_FUNC real
inverseDistance(const ivec3_t &a, const ivec3_t &b) PURE_FUNC;

constexpr MATH_FUNC defs::int32
distanceSq(const ivec3_t &a, const ivec3_t &b) PURE_FUNC;

constexpr MATH_FUNC ivec3_t
min(const ivec3_t &a, const ivec3_t &max) PURE_FUNC;

constexpr MATH_FUNC ivec3_t
max(const ivec3_t &a, const ivec3_t &max) PURE_FUNC;

constexpr MATH_FUNC defs::int32
sum(const ivec3_t &a) PURE_FUNC;

constexpr MATH_FUNC bool
equal(const ivec3_t &a, const ivec3_t &b) PURE_FUNC;

MATH_END_NAMESPACE

#endif
