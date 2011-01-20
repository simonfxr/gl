#if defined(MATH_VEC4_INLINE) || !defined(MATH_INLINE)

#include "math/math.hpp"
#include "math/vec4.hpp"

MATH_BEGIN_NAMESPACE

vec4_t vec4(float x, float y, float z, float w) {
    vec4_t v; v.x = x; v.y = y; v.z = z; v.w = w; return v;
}

vec4_t vec4(float a) {
    return vec4(a, a, a, a);
}

vec4_t vec4(const vec3_t& a, float w) {
    return vec4(a.x, a.y, a.z, w);
}

vec4_t vec4(const float a[4]) {
    return vec4(a[0], a[1], a[2], a[3]);
}

vec4_t operator -(const vec4_t& a) {
    return vec4(0.f) - a;
}

vec4_t operator +(const vec4_t& a, const vec4_t b) {
    return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

vec4_t operator -(const vec4_t& a, const vec4_t b) {
    return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

vec4_t operator *(const vec4_t& v, float a) {
    return v * vec4(a);
}

vec4_t operator *(float a, const vec4_t& v) {
    return v * a;
}

vec4_t operator *(const vec4_t& a, const vec4_t& b) {
    return vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

vec4_t operator /(const vec4_t& v, float a) {
    return v * math::recp(a);
}

vec4_t& operator +=(vec4_t& v, const vec4_t& a) {
    return v = v + a;
}

vec4_t& operator -=(vec4_t& v, const vec4_t& a) {
    return v = v - a;
}

vec4_t& operator *=(vec4_t& v, float a) {
    return v = v * a;
}

vec4_t& operator *=(vec4_t& v, const vec4_t& b) {
    return v = v * b;
}

vec4_t& operator /=(vec4_t& v, float a) {
    return v = v / a;
}

bool operator ==(vec4_t& a, vec4_t& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

bool operator !=(vec4_t& a, vec4_t& b) {
    return !(a == b);
}

#ifndef MATH_INLINE

float& vec4_t::operator[](unsigned long i) {
    return components[i];
}

float vec4_t::operator[](unsigned long i) const {
    return components[i];
}

#endif

float dot(const vec4_t& a, const vec4_t& b) {
    return sum(a * b);
}

float length(const vec4_t& a) {
    return math::sqrt(lengthSq(a));
}

float inverseLength(const vec4_t& a) {
    return math::rsqrt(lengthSq(a));
}

float lengthSq(const vec4_t& a) {
    return dot(a, a);
}

vec4_t normalize(const vec4_t& a) {
    return a * inverseLength(a);
}

float distance(const vec4_t& a, const vec4_t& b) {
    return length(a - b);
}

float inverseDistance(const vec4_t& a, const vec4_t& b) {
    return inverseLength(a - b);
}

float distanceSq(const vec4_t& a, const vec4_t& b) {
    return lengthSq(a - b);
}

vec4_t min(const vec4_t& a, const vec4_t& b) {
    return vec4(b.x < a.x ? b.x : a.x,
                b.y < a.y ? b.y : a.y,
                b.z < a.z ? b.z : a.z,
                b.w < a.w ? b.w : a.w);
}

vec4_t max(const vec4_t& a, const vec4_t& b) {
    return vec4(b.x > a.x ? b.x : a.x,
                b.y > a.y ? b.y : a.y,
                b.z > a.z ? b.z : a.z,
                b.w > a.w ? b.w : a.w);
}

float sum(const vec4_t a) {
    return a.x + a.y + a.z + a.w;
}

MATH_END_NAMESPACE

#endif
