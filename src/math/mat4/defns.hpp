#ifndef MAT4_DEFNS_HPP
#define MAT4_DEFNS_HPP

#include "math/mat3/type.hpp"
#include "math/mat4/type.hpp"
#include "math/vec3.hpp"

namespace math {

constexpr inline mat4_t
mat4();

constexpr inline mat4_t
mat4(real x);

constexpr inline mat4_t
mat4(const real mat[16]);

constexpr inline mat4_t
mat4(const mat3_t &m);

constexpr inline mat4_t
mat4(const vec4_t &c1, const vec4_t &c2, const vec4_t &c3, const vec4_t &c4);

constexpr inline void
load(mat4_t::buffer, const mat4_t &);

constexpr inline mat4_t
operator+(const mat4_t &A, const mat4_t &B);

constexpr inline mat4_t
operator-(const mat4_t &A, const mat4_t &B);

constexpr inline mat4_t operator*(const mat4_t &A, const mat4_t &B);

constexpr inline vec4_t operator*(const mat4_t &A, const vec4_t &v);

constexpr inline mat4_t operator*(const mat4_t &A, real x);

constexpr inline mat4_t operator*(real x, const mat4_t &A);

constexpr inline mat4_t
operator/(const mat4_t &A, real x);

constexpr inline mat4_t &
operator+=(mat4_t &A, const mat4_t &B);

constexpr inline mat4_t &
operator-=(mat4_t &A, const mat4_t &B);

constexpr inline mat4_t &
operator*=(mat4_t &A, real x);

constexpr inline mat4_t &
operator*=(mat4_t &A, const mat4_t &B);

constexpr inline mat4_t &
operator/=(mat4_t &A, real x);

inline real
determinant(const mat4_t &A);

inline mat4_t
inverse(const mat4_t &A);

constexpr inline vec4_t
transform(const mat4_t &A, const vec4_t &v);

constexpr inline vec3_t
transformPoint(const mat4_t &A, const point3_t &p);

constexpr inline vec3_t
transformVector(const mat4_t &A, const vec3_t &v);

constexpr inline mat4_t
transpose(const mat4_t &A);

constexpr inline vec4_t
transposedMult(const mat4_t &AT, const vec4_t &v);

inline bool
equal(const mat4_t &A, const mat4_t &B, real epsi = real(1e-4));

} // namespace math

#endif
