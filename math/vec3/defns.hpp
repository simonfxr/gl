#ifndef VEC3_DEFNS_HPP
#define VEC3_DEFNS_HPP

#include "math/defs.hpp"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"

MATH_BEGIN_NAMESPACE

vec3_t PURE_FUNC vec3(float x, float y, float z);

vec3_t PURE_FUNC vec3(float a);

vec3_t PURE_FUNC vec3(const vec4_t& a);

vec3_t PURE_FUNC vec3(const float a[3]);

vec3_t PURE_FUNC operator -(const vec3_t& a);

vec3_t PURE_FUNC operator +(const vec3_t& a, const vec3_t b);

vec3_t PURE_FUNC operator -(const vec3_t& a, const vec3_t b);

vec3_t PURE_FUNC operator *(const vec3_t& v, float a);

vec3_t PURE_FUNC operator *(float a, const vec3_t& v);

vec3_t PURE_FUNC operator *(const vec3_t& a, const vec3_t& b);

vec3_t PURE_FUNC operator /(const vec3_t& v, float a);

vec3_t& MUT_FUNC operator +=(vec3_t& v, const vec3_t& a);

vec3_t& MUT_FUNC operator -=(vec3_t& v, const vec3_t& a);

vec3_t& MUT_FUNC operator *=(vec3_t& v, float a);

vec3_t& MUT_FUNC operator *=(vec3_t& v, const vec3_t& b);

vec3_t& MUT_FUNC operator /=(vec3_t& v, float a);

bool PURE_FUNC operator ==(vec3_t& a, vec3_t& b);

bool PURE_FUNC operator !=(vec3_t& a, vec3_t& b);

float PURE_FUNC dot(const vec3_t& a, const vec3_t& b);

vec3_t PURE_FUNC cross(const vec3_t& a, const vec3_t& b);

float PURE_FUNC length(const vec3_t& a);

float PURE_FUNC inverseLength(const vec3_t& a);

float PURE_FUNC lengthSq(const vec3_t& a);

vec3_t PURE_FUNC normalize(const vec3_t& a);

float PURE_FUNC distance(const vec3_t& a, const vec3_t& b);

float PURE_FUNC inverseDistance(const vec3_t& a, const vec3_t& b);

float PURE_FUNC distanceSq(const vec3_t& a, const vec3_t& b);

vec3_t PURE_FUNC reflect(const vec3_t& a, const vec3_t& normal);

vec3_t PURE_FUNC reflect(const vec3_t& a, const vec3_t& normal, float amp);

vec3_t PURE_FUNC min(const vec3_t& a, const vec3_t& max);

vec3_t PURE_FUNC max(const vec3_t& a, const vec3_t& max);

float PURE_FUNC sum(const vec3_t a);

MATH_END_NAMESPACE

#endif
