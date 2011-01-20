#ifndef VEC4_DEFNS_HPP
#define VEC4_DEFNS_HPP

#include "math/defs.hpp"
#include "math/vec4/type.hpp"
#include "math/vec3/type.hpp"

MATH_BEGIN_NAMESPACE

vec4_t PURE_FUNC vec4(float x, float y, float z, float w);

vec4_t PURE_FUNC vec4(float a);

vec4_t PURE_FUNC vec4(const vec3_t& a, float w);

vec4_t PURE_FUNC vec4(const float a[4]);

vec4_t PURE_FUNC operator -(const vec4_t& a);

vec4_t PURE_FUNC operator +(const vec4_t& a, const vec4_t b);

vec4_t PURE_FUNC operator -(const vec4_t& a, const vec4_t b);

vec4_t PURE_FUNC operator *(const vec4_t& v, float a);

vec4_t PURE_FUNC operator *(float a, const vec4_t& v);

vec4_t PURE_FUNC operator *(const vec4_t& a, const vec4_t& b);

vec4_t PURE_FUNC operator /(const vec4_t& v, float a);

vec4_t& MUT_FUNC operator +=(vec4_t& v, const vec4_t& a);

vec4_t& MUT_FUNC operator -=(vec4_t& v, const vec4_t& a);

vec4_t& MUT_FUNC operator *=(vec4_t& v, float a);

vec4_t& MUT_FUNC operator *=(vec4_t& v, const vec4_t& b);

vec4_t& MUT_FUNC operator /=(vec4_t& v, float a);

bool PURE_FUNC operator ==(vec4_t& a, vec4_t& b);

bool PURE_FUNC operator !=(vec4_t& a, vec4_t& b);

float PURE_FUNC dot(const vec4_t& a, const vec4_t& b);

float PURE_FUNC length(const vec4_t& a);

float PURE_FUNC inverseLength(const vec4_t& a);

float PURE_FUNC lengthSq(const vec4_t& a);

vec4_t PURE_FUNC normalize(const vec4_t& a);

float PURE_FUNC distance(const vec4_t& a, const vec4_t& b);

float PURE_FUNC inverseDistance(const vec4_t& a, const vec4_t& b);

float PURE_FUNC distanceSq(const vec4_t& a, const vec4_t& b);

vec4_t PURE_FUNC min(const vec4_t& a, const vec4_t& max);

vec4_t PURE_FUNC max(const vec4_t& a, const vec4_t& max);

float PURE_FUNC sum(const vec4_t a);

MATH_END_NAMESPACE

#endif
