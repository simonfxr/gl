#ifndef MATH_IVEC3_DEFNS_HPP
#define MATH_IVEC3_DEFNS_HPP

#include "math/ivec3/type.hpp"
#include "math/vec3/type.hpp"

namespace math {

constexpr inline ivec3_t
ivec3(int32_t x, int32_t y, int32_t z);

constexpr inline ivec3_t
ivec3(int32_t a);

constexpr inline ivec3_t
ivec3(const vec3_t &a);

constexpr inline ivec3_t
ivec3(const int32_t a[3]);

constexpr inline ivec3_t
operator-(const ivec3_t &a);

constexpr inline ivec3_t
operator+(const ivec3_t &a, const ivec3_t b);

constexpr inline ivec3_t
operator-(const ivec3_t &a, const ivec3_t b);

constexpr inline ivec3_t operator*(const ivec3_t &v, int32_t a);

constexpr inline ivec3_t operator*(int32_t a, const ivec3_t &v);

constexpr inline ivec3_t operator*(const ivec3_t &a, const ivec3_t &b);

constexpr inline ivec3_t
operator/(const ivec3_t &v, int32_t a);

constexpr inline ivec3_t
operator/(const ivec3_t &v, const ivec3_t &b);

constexpr inline ivec3_t &
operator+=(ivec3_t &v, const ivec3_t &a);

constexpr inline ivec3_t &
operator-=(ivec3_t &v, const ivec3_t &a);

constexpr inline ivec3_t &
operator*=(ivec3_t &v, int32_t a);

constexpr inline ivec3_t &
operator*=(ivec3_t &v, const ivec3_t &b);

constexpr inline ivec3_t &
operator/=(ivec3_t &v, int32_t a);

constexpr inline ivec3_t &
operator/=(ivec3_t &v, const ivec3_t &b);

constexpr inline bool
operator==(const ivec3_t &a, const ivec3_t &b);

constexpr inline bool
operator!=(const ivec3_t &a, const ivec3_t &b);

constexpr inline int32_t
dot(const ivec3_t &a, const ivec3_t &b);

constexpr inline ivec3_t
cross(const ivec3_t &a, const ivec3_t &b);

inline real
length(const ivec3_t &a);

inline real
inverseLength(const ivec3_t &a);

constexpr inline int32_t
lengthSq(const ivec3_t &a);

inline real
distance(const ivec3_t &a, const ivec3_t &b);

inline real
inverseDistance(const ivec3_t &a, const ivec3_t &b);

constexpr inline int32_t
distanceSq(const ivec3_t &a, const ivec3_t &b);

constexpr inline ivec3_t
min(const ivec3_t &a, const ivec3_t &max);

constexpr inline ivec3_t
max(const ivec3_t &a, const ivec3_t &max);

constexpr inline int32_t
sum(const ivec3_t &a);

constexpr inline bool
equal(const ivec3_t &a, const ivec3_t &b);

} // namespace math

#endif
