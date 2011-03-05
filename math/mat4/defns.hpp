#ifndef MAT4_DEFNS_HPP
#define MAT4_DEFNS_HPP

#include "math/mat4/type.hpp"

MATH_BEGIN_NAMESPACE

struct vec3_t;

mat4_t mat4() PURE_FUNC;

mat4_t mat4(float x) PURE_FUNC;

mat4_t mat4(const float mat[16]) PURE_FUNC;

mat4_t mat4(const vec4_t& c1, const vec4_t& c2, const vec4_t& c3, const vec4_t& c4) PURE_FUNC;

mat4_t operator +(const mat4_t& A, const mat4_t& B) PURE_FUNC;

mat4_t operator -(const mat4_t& A, const mat4_t& B) PURE_FUNC;

mat4_t operator *(const mat4_t& A, const mat4_t& B) PURE_FUNC;

vec4_t operator *(const mat4_t& A, const vec4_t& v) PURE_FUNC;

mat4_t operator *(const mat4_t& A, float x) PURE_FUNC;

mat4_t operator *(float x, const mat4_t& A) PURE_FUNC;

mat4_t operator /(const mat4_t& A, float x) PURE_FUNC;

mat4_t& operator +=(mat4_t& A, const mat4_t& B) MUT_FUNC;

mat4_t& operator -=(mat4_t& A, const mat4_t& B) MUT_FUNC;

mat4_t& operator *=(mat4_t& A, float x) MUT_FUNC;

mat4_t& operator *=(mat4_t& A, const mat4_t& B) MUT_FUNC;

mat4_t& operator /=(mat4_t& A, float x) MUT_FUNC;

vec4_t transform(const mat4_t& A, const vec4_t& v) PURE_FUNC;

vec3_t transformPoint(const mat4_t& A, const vec3_t& p) PURE_FUNC;

vec3_t transformVector(const mat4_t& A, const vec3_t& v) PURE_FUNC;

mat4_t transpose(const mat4_t& A) PURE_FUNC;

mat4_t inverse(const mat4_t& A) PURE_FUNC;

vec4_t transposedMult(const mat4_t& AT, const vec4_t& v) PURE_FUNC;

MATH_END_NAMESPACE

#endif
