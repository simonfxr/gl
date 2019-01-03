#ifndef MAT3_DEFNS_HPP
#define MAT3_DEFNS_HPP

#include "math/mat3/type.hpp"
#include "math/mat4/type.hpp"

namespace math {

constexpr inline mat3_t
mat3();

constexpr inline mat3_t
mat3(real x);

constexpr inline mat3_t
mat3(const mat3_t::buffer);

constexpr inline mat3_t
mat3(const vec3_t &c1, const vec3_t &c2, const vec3_t &c3);

constexpr inline mat3_t
mat3(const mat4_t &A);

constexpr inline void
load(mat3_t::buffer, const mat3_t &);

constexpr inline mat3_t
operator+(const mat3_t &A, const mat3_t &B);

constexpr inline mat3_t
operator-(const mat3_t &A, const mat3_t &B);

constexpr inline mat3_t operator*(const mat3_t &A, const mat3_t &B);

constexpr inline vec3_t operator*(const mat3_t &A, const vec3_t &v);

constexpr inline mat3_t operator*(const mat3_t &A, real x);

constexpr inline mat3_t operator*(real x, const mat3_t &A);

constexpr inline mat3_t
operator/(const mat3_t &A, real x);

constexpr inline mat3_t &
operator+=(mat3_t &A, const mat3_t &B);

constexpr inline mat3_t &
operator-=(mat3_t &A, const mat3_t &B);

constexpr inline mat3_t &
operator*=(mat3_t &A, real x);

constexpr inline mat3_t &
operator*=(mat3_t &A, const mat3_t &B);

constexpr inline mat3_t &
operator/=(mat3_t &A, real x);

constexpr inline real
determinant(const mat3_t &A);

inline mat3_t
inverse(const mat3_t &A);

inline mat3_t
orthonormalBasis(const mat3_t &A);

constexpr inline vec3_t
transform(const mat3_t &A, const vec3_t &v);

constexpr inline point3_t
transformPoint(const mat3_t &A, const point3_t &v);

constexpr inline vec3_t
transformVector(const mat3_t &A, const vec3_t &v);

constexpr inline mat3_t
transpose(const mat3_t &A);

inline bool
equal(const mat3_t &A, const mat3_t &B, real epsi = real(1e-4));

inline mat3_t
coordinateSystem(const vec3_t &a);

} // namespace math

#endif
