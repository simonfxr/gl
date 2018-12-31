#ifndef MAT4_DEFNS_HPP
#define MAT4_DEFNS_HPP

#include "math/mat3/type.hpp"
#include "math/mat4/type.hpp"
#include "math/vec3/type.hpp"

MATH_BEGIN_NAMESPACE

constexpr MATH_FUNC mat4_t
mat4() PURE_FUNC;

constexpr MATH_FUNC mat4_t
mat4(real x) PURE_FUNC;

constexpr MATH_FUNC mat4_t
mat4(const real mat[16]) PURE_FUNC;

constexpr MATH_FUNC mat4_t
mat4(const mat3_t &m) PURE_FUNC;

constexpr MATH_FUNC mat4_t
mat4(const vec4_t &c1,
     const vec4_t &c2,
     const vec4_t &c3,
     const vec4_t &c4) PURE_FUNC;

MATH_FUNC void
load(mat4_t::buffer, const mat4_t &) MUT_FUNC;

constexpr MATH_FUNC mat4_t
operator+(const mat4_t &A, const mat4_t &B) PURE_FUNC;

constexpr MATH_FUNC mat4_t
operator-(const mat4_t &A, const mat4_t &B) PURE_FUNC;

constexpr MATH_FUNC mat4_t operator*(const mat4_t &A,
                                     const mat4_t &B) PURE_FUNC;

constexpr MATH_FUNC vec4_t operator*(const mat4_t &A,
                                     const vec4_t &v) PURE_FUNC;

constexpr MATH_FUNC mat4_t operator*(const mat4_t &A, real x) PURE_FUNC;

constexpr MATH_FUNC mat4_t operator*(real x, const mat4_t &A) PURE_FUNC;

constexpr MATH_FUNC mat4_t
operator/(const mat4_t &A, real x) PURE_FUNC;

constexpr MATH_FUNC mat4_t &
operator+=(mat4_t &A, const mat4_t &B) MUT_FUNC;

constexpr MATH_FUNC mat4_t &
operator-=(mat4_t &A, const mat4_t &B) MUT_FUNC;

constexpr MATH_FUNC mat4_t &
operator*=(mat4_t &A, real x) MUT_FUNC;

constexpr MATH_FUNC mat4_t &
operator*=(mat4_t &A, const mat4_t &B) MUT_FUNC;

constexpr MATH_FUNC mat4_t &
operator/=(mat4_t &A, real x) MUT_FUNC;

MATH_FUNC real
determinant(const mat4_t &A) PURE_FUNC;

MATH_FUNC mat4_t
inverse(const mat4_t &A) PURE_FUNC;

constexpr MATH_FUNC vec4_t
transform(const mat4_t &A, const vec4_t &v) PURE_FUNC;

constexpr MATH_FUNC vec3_t
transformPoint(const mat4_t &A, const point3_t &p) PURE_FUNC;

constexpr MATH_FUNC vec3_t
transformVector(const mat4_t &A, const vec3_t &v) PURE_FUNC;

constexpr MATH_FUNC mat4_t
transpose(const mat4_t &A) PURE_FUNC;

constexpr MATH_FUNC vec4_t
transposedMult(const mat4_t &AT, const vec4_t &v) PURE_FUNC;

constexpr MATH_FUNC bool
equal(const mat4_t &A, const mat4_t &B, real epsi = real(1e-4)) PURE_FUNC;

MATH_END_NAMESPACE

#endif
