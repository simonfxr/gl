#ifndef VEC3_DEFNS_HPP
#define VEC3_DEFNS_HPP

#include "math/defs.hpp"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"

MATH_BEGIN_NAMESPACE

vec3_t vec3(float x, float y, float z) PURE_FUNC;

vec3_t vec3(float a) PURE_FUNC;

vec3_t vec3(const vec4_t& a) PURE_FUNC;

vec3_t vec3(const float a[3]) PURE_FUNC;

vec3_t operator -(const vec3_t& a) PURE_FUNC;

vec3_t operator +(const vec3_t& a, const vec3_t b) PURE_FUNC;

vec3_t operator -(const vec3_t& a, const vec3_t b) PURE_FUNC;

vec3_t operator *(const vec3_t& v, float a) PURE_FUNC;

vec3_t operator *(float a, const vec3_t& v) PURE_FUNC;

vec3_t operator *(const vec3_t& a, const vec3_t& b) PURE_FUNC;

vec3_t operator /(const vec3_t& v, float a) PURE_FUNC;

vec3_t& operator +=(vec3_t& v, const vec3_t& a) MUT_FUNC;

vec3_t& operator -=(vec3_t& v, const vec3_t& a) MUT_FUNC;

vec3_t& operator *=(vec3_t& v, float a) MUT_FUNC;

vec3_t& operator *=(vec3_t& v, const vec3_t& b) MUT_FUNC;

vec3_t& operator /=(vec3_t& v, float a) MUT_FUNC;

bool operator ==(vec3_t& a, vec3_t& b) PURE_FUNC;

bool operator !=(vec3_t& a, vec3_t& b) PURE_FUNC;

float dot(const vec3_t& a, const vec3_t& b) PURE_FUNC;

vec3_t cross(const vec3_t& a, const vec3_t& b) PURE_FUNC;

float length(const vec3_t& a) PURE_FUNC;

float inverseLength(const vec3_t& a) PURE_FUNC;

float lengthSq(const vec3_t& a) PURE_FUNC;

vec3_t normalize(const vec3_t& a) PURE_FUNC;

float distance(const vec3_t& a, const vec3_t& b) PURE_FUNC;

float inverseDistance(const vec3_t& a, const vec3_t& b) PURE_FUNC;

float distanceSq(const vec3_t& a, const vec3_t& b) PURE_FUNC;

vec3_t reflect(const vec3_t& a, const vec3_t& normal) PURE_FUNC;

vec3_t reflect(const vec3_t& a, const vec3_t& normal, float amp) PURE_FUNC;

vec3_t min(const vec3_t& a, const vec3_t& max) PURE_FUNC;

vec3_t max(const vec3_t& a, const vec3_t& max) PURE_FUNC;

float sum(const vec3_t& a) PURE_FUNC;

vec3_t linearInterpolate(const vec3_t& a, const vec3_t& b, float t) PURE_FUNC;

MATH_END_NAMESPACE

#endif
