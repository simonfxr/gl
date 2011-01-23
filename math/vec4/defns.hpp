#ifndef VEC4_DEFNS_HPP
#define VEC4_DEFNS_HPP

#include "math/defs.hpp"
#include "math/vec4/type.hpp"
#include "math/vec3/type.hpp"

MATH_BEGIN_NAMESPACE

vec4_t vec4(float x, float y, float z, float w) PURE_FUNC;

vec4_t vec4(float a) PURE_FUNC;

vec4_t vec4(const vec3_t& a, float w) PURE_FUNC;

vec4_t vec4(const float a[4]) PURE_FUNC;

#if MATH_SSE(2, 0)
vec4_t vec4(__m128 xmm) PURE_FUNC;
#endif

vec4_t operator -(const vec4_t& a) PURE_FUNC;

vec4_t operator +(const vec4_t& a, const vec4_t b) PURE_FUNC;

vec4_t operator -(const vec4_t& a, const vec4_t b) PURE_FUNC;

vec4_t operator *(const vec4_t& v, float a) PURE_FUNC;

vec4_t operator *(float a, const vec4_t& v) PURE_FUNC;

vec4_t operator *(const vec4_t& a, const vec4_t& b) PURE_FUNC;

vec4_t operator /(const vec4_t& v, float a) PURE_FUNC;

vec4_t& operator +=(vec4_t& v, const vec4_t& a) MUT_FUNC;

vec4_t& operator -=(vec4_t& v, const vec4_t& a) MUT_FUNC;

vec4_t& operator *=(vec4_t& v, float a) MUT_FUNC;

vec4_t& operator *=(vec4_t& v, const vec4_t& b) MUT_FUNC;

vec4_t& operator /=(vec4_t& v, float a) MUT_FUNC;

bool operator ==(vec4_t& a, vec4_t& b) PURE_FUNC;

bool operator !=(vec4_t& a, vec4_t& b) PURE_FUNC;

float dot(const vec4_t& a, const vec4_t& b) PURE_FUNC;

float length(const vec4_t& a) PURE_FUNC;

float inverseLength(const vec4_t& a) PURE_FUNC;

float lengthSq(const vec4_t& a) PURE_FUNC;

vec4_t normalize(const vec4_t& a) PURE_FUNC;

float distance(const vec4_t& a, const vec4_t& b) PURE_FUNC;

float inverseDistance(const vec4_t& a, const vec4_t& b) PURE_FUNC;

float distanceSq(const vec4_t& a, const vec4_t& b) PURE_FUNC;

vec4_t min(const vec4_t& a, const vec4_t& max) PURE_FUNC;

vec4_t max(const vec4_t& a, const vec4_t& max) PURE_FUNC;

float sum(const vec4_t& a) PURE_FUNC;

MATH_END_NAMESPACE

#endif
