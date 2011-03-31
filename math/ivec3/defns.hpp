#ifndef MATH_IVEC3_DEFNS_HPP
#define MATH_IVEC3_DEFNS_HPP

#include "math/defs.hpp"
#include "math/ivec3/type.hpp"
#include "math/vec3/type.hpp"

MATH_BEGIN_NAMESPACE

ivec3_t ivec3(int32 x, int32 y, int32 z) PURE_FUNC;

ivec3_t ivec3(int32 a) PURE_FUNC;

ivec3_t ivec3(const vec3_t& a) PURE_FUNC;

ivec3_t ivec3(const int32 a[3]) PURE_FUNC;

ivec3_t operator -(const ivec3_t& a) PURE_FUNC;

ivec3_t operator +(const ivec3_t& a, const ivec3_t b) PURE_FUNC;

ivec3_t operator -(const ivec3_t& a, const ivec3_t b) PURE_FUNC;

ivec3_t operator *(const ivec3_t& v, int32 a) PURE_FUNC;

ivec3_t operator *(int32 a, const ivec3_t& v) PURE_FUNC;

ivec3_t operator *(const ivec3_t& a, const ivec3_t& b) PURE_FUNC;

ivec3_t operator /(const ivec3_t& v, int32 a) PURE_FUNC;

ivec3_t operator /(const ivec3_t& v, const ivec3_t& b) PURE_FUNC;

ivec3_t& operator +=(ivec3_t& v, const ivec3_t& a) MUT_FUNC;

ivec3_t& operator -=(ivec3_t& v, const ivec3_t& a) MUT_FUNC;

ivec3_t& operator *=(ivec3_t& v, int32 a) MUT_FUNC;

ivec3_t& operator *=(ivec3_t& v, const ivec3_t& b) MUT_FUNC;

ivec3_t& operator /=(ivec3_t& v, int32 a) MUT_FUNC;

ivec3_t& operator /=(ivec3_t& v, const ivec3_t& b) MUT_FUNC;

bool operator ==(const ivec3_t& a, const ivec3_t& b) PURE_FUNC;

bool operator !=(const ivec3_t& a, const ivec3_t& b) PURE_FUNC;

int32 dot(const ivec3_t& a, const ivec3_t& b) PURE_FUNC;

ivec3_t cross(const ivec3_t& a, const ivec3_t& b) PURE_FUNC;

float length(const ivec3_t& a) PURE_FUNC;

float inverseLength(const ivec3_t& a) PURE_FUNC;

int32 lengthSq(const ivec3_t& a) PURE_FUNC;

float distance(const ivec3_t& a, const ivec3_t& b) PURE_FUNC;

float inverseDistance(const ivec3_t& a, const ivec3_t& b) PURE_FUNC;

int32 distanceSq(const ivec3_t& a, const ivec3_t& b) PURE_FUNC;

ivec3_t min(const ivec3_t& a, const ivec3_t& max) PURE_FUNC;

ivec3_t max(const ivec3_t& a, const ivec3_t& max) PURE_FUNC;

int32 sum(const ivec3_t& a) PURE_FUNC;

bool equal(const ivec3_t& a, const ivec3_t& b) PURE_FUNC;

MATH_END_NAMESPACE

#endif
