#ifndef MAT3_DEFNS_HPP
#define MAT3_DEFNS_HPP

#include "math/mat3/type.hpp"
#include "math/mat4/type.hpp"

MATH_BEGIN_NAMESPACE

constexpr MATH_FUNC mat3_t
mat3() PURE_FUNC;

constexpr MATH_FUNC mat3_t
mat3(real x) PURE_FUNC;

constexpr MATH_FUNC mat3_t
mat3(const mat3_t::buffer) PURE_FUNC;

constexpr MATH_FUNC mat3_t
mat3(const vec3_t &c1, const vec3_t &c2, const vec3_t &c3) PURE_FUNC;

constexpr MATH_FUNC mat3_t
mat3(const mat4_t &A) PURE_FUNC;

constexpr MATH_FUNC void
load(mat3_t::buffer, const mat3_t &) MUT_FUNC;

constexpr MATH_FUNC mat3_t
operator+(const mat3_t &A, const mat3_t &B) PURE_FUNC;

constexpr MATH_FUNC mat3_t
operator-(const mat3_t &A, const mat3_t &B) PURE_FUNC;

constexpr MATH_FUNC mat3_t operator*(const mat3_t &A,
                                     const mat3_t &B) PURE_FUNC;

constexpr MATH_FUNC vec3_t operator*(const mat3_t &A,
                                     const vec3_t &v) PURE_FUNC;

constexpr MATH_FUNC mat3_t operator*(const mat3_t &A, real x) PURE_FUNC;

constexpr MATH_FUNC mat3_t operator*(real x, const mat3_t &A) PURE_FUNC;

constexpr MATH_FUNC mat3_t
operator/(const mat3_t &A, real x) PURE_FUNC;

constexpr MATH_FUNC mat3_t &
operator+=(mat3_t &A, const mat3_t &B) MUT_FUNC;

constexpr MATH_FUNC mat3_t &
operator-=(mat3_t &A, const mat3_t &B) MUT_FUNC;

constexpr MATH_FUNC mat3_t &
operator*=(mat3_t &A, real x) MUT_FUNC;

constexpr MATH_FUNC mat3_t &
operator*=(mat3_t &A, const mat3_t &B) MUT_FUNC;

constexpr MATH_FUNC mat3_t &
operator/=(mat3_t &A, real x) MUT_FUNC;

constexpr MATH_FUNC real
determinant(const mat3_t &A) PURE_FUNC;

MATH_FUNC mat3_t
inverse(const mat3_t &A) PURE_FUNC;

MATH_FUNC mat3_t
orthonormalBasis(const mat3_t &A) PURE_FUNC;

constexpr MATH_FUNC vec3_t
transform(const mat3_t &A, const vec3_t &v) PURE_FUNC;

constexpr MATH_FUNC point3_t
transformPoint(const mat3_t &A, const point3_t &v) PURE_FUNC;

constexpr MATH_FUNC vec3_t
transformVector(const mat3_t &A, const vec3_t &v) PURE_FUNC;

constexpr MATH_FUNC mat3_t
transpose(const mat3_t &A) PURE_FUNC;

MATH_FUNC bool
equal(const mat3_t &A, const mat3_t &B, real epsi = real(1e-4)) PURE_FUNC;

constexpr MATH_FUNC mat3_t
coordinateSystem(const vec3_t &a) PURE_FUNC;

MATH_END_NAMESPACE

#endif
