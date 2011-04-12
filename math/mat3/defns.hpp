#ifndef MAT3_DEFNS_HPP
#define MAT3_DEFNS_HPP

#include "math/mat3/type.hpp"

MATH_BEGIN_NAMESPACE

mat3_t mat3() PURE_FUNC;

mat3_t mat3(float x) PURE_FUNC;

mat3_t mat3(const float mat[9]) PURE_FUNC;

mat3_t mat3(const vec3_t& c1, const vec3_t& c2, const vec3_t& c3) PURE_FUNC;

mat3_t operator +(const mat3_t& A, const mat3_t& B) PURE_FUNC;

mat3_t operator -(const mat3_t& A, const mat3_t& B) PURE_FUNC;

mat3_t operator *(const mat3_t& A, const mat3_t& B) PURE_FUNC;

vec3_t operator *(const mat3_t& A, const vec3_t& v) PURE_FUNC;

mat3_t operator *(const mat3_t& A, float x) PURE_FUNC;

mat3_t operator *(float x, const mat3_t& A) PURE_FUNC;

mat3_t operator /(const mat3_t& A, float x) PURE_FUNC;

mat3_t& operator +=(mat3_t& A, const mat3_t& B) MUT_FUNC;

mat3_t& operator -=(mat3_t& A, const mat3_t& B) MUT_FUNC;

mat3_t& operator *=(mat3_t& A, float x) MUT_FUNC;

mat3_t& operator *=(mat3_t& A, const mat3_t& B) MUT_FUNC;

mat3_t& operator /=(mat3_t& A, float x) MUT_FUNC;

float determinant(const mat3_t& A) PURE_FUNC;

mat3_t inverse(const mat3_t& A) PURE_FUNC;

vec3_t transform(const mat3_t& A, const vec3_t& v) PURE_FUNC;

point3_t transformPoint(const mat3_t& A, const point3_t& v) PURE_FUNC;

vec3_t transformVector(const mat3_t& A, const vec3_t& v) PURE_FUNC;

mat3_t transpose(const mat3_t& A) PURE_FUNC;

bool equal(const mat3_t& A, const mat3_t& B, float epsi = 1e-4) PURE_FUNC;

MATH_END_NAMESPACE

#endif
