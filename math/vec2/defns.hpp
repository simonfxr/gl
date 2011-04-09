#ifndef MATH_VEC2_DEFNS_HPP
#define MATH_VEC2_DEFNS_HPP

#include "math/defs.hpp"
#include "math/vec2/type.hpp"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"

MATH_BEGIN_NAMESPACE

vec2_t vec2(float x, float y) PURE_FUNC;

vec2_t vec2(float a) PURE_FUNC;

vec2_t vec2(const vec3_t& a) PURE_FUNC;

vec2_t vec2(const vec4_t& a) PURE_FUNC;

vec2_t vec2(const float a[2]) PURE_FUNC;

vec2_t operator -(const vec2_t& a) PURE_FUNC;

vec2_t operator +(const vec2_t& a, const vec2_t b) PURE_FUNC;

vec2_t operator -(const vec2_t& a, const vec2_t b) PURE_FUNC;

vec2_t operator *(const vec2_t& v, float a) PURE_FUNC;

vec2_t operator *(float a, const vec2_t& v) PURE_FUNC;

vec2_t operator *(const vec2_t& a, const vec2_t& b) PURE_FUNC;

vec2_t operator /(const vec2_t& v, float a) PURE_FUNC;

vec2_t& operator +=(vec2_t& v, const vec2_t& a) MUT_FUNC;

vec2_t& operator -=(vec2_t& v, const vec2_t& a) MUT_FUNC;

vec2_t& operator *=(vec2_t& v, float a) MUT_FUNC;

vec2_t& operator *=(vec2_t& v, const vec2_t& b) MUT_FUNC;

vec2_t& operator /=(vec2_t& v, float a) MUT_FUNC;

bool operator ==(const vec2_t& a, const vec2_t& b) PURE_FUNC;

bool operator !=(const vec2_t& a, const vec2_t& b) PURE_FUNC;

float dot(const vec2_t& a, const vec2_t& b) PURE_FUNC;

float length(const vec2_t& a) PURE_FUNC;

float inverseLength(const vec2_t& a) PURE_FUNC;

float lengthSq(const vec2_t& a) PURE_FUNC;

direction2_t normalize(const vec2_t& a) PURE_FUNC;

float distance(const vec2_t& a, const vec2_t& b) PURE_FUNC;

float inverseDistance(const vec2_t& a, const vec2_t& b) PURE_FUNC;

float distanceSq(const vec2_t& a, const vec2_t& b) PURE_FUNC;

vec2_t reflect(const vec2_t& a, const normal2_t& d) PURE_FUNC;

vec2_t reflect(const vec2_t& a, const normal2_t& d, float amp) PURE_FUNC;

vec2_t min(const vec2_t& a, const vec2_t& max) PURE_FUNC;

vec2_t max(const vec2_t& a, const vec2_t& max) PURE_FUNC;

float sum(const vec2_t& a) PURE_FUNC;

vec2_t linearInterpolate(const vec2_t& a, const vec2_t& b, float t) PURE_FUNC;

direction2_t directionFromTo(const point2_t& a, const point2_t& b) PURE_FUNC;

float cos(const vec2_t& a, const vec2_t& b) PURE_FUNC;

vec2_t projectAlong(const vec2_t& a, const vec2_t& x) PURE_FUNC;

bool equal(const vec2_t& a, const vec2_t& b, float epsi = 1e-4) PURE_FUNC;

MATH_END_NAMESPACE

#endif
