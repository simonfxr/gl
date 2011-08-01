#include "math/defs.hpp"

#if defined(MATH_VEC4_INLINE) || !defined(MATH_INLINE)

#include "math/real.hpp"
#include "math/vec4.hpp"

MATH_BEGIN_NAMESPACE

vec4_t vec4(float x, float y, float z, float w) {
    vec4_t v; v[0] = x; v[1] = y; v[2] = z; v[3] = w; return v;
}

vec4_t vec4(float a) {
    return vec4(a, a, a, a);
}

vec4_t vec4(const vec3_t& a, float w) {
    return vec4(a[0], a[1], a[2], w);
}

vec4_t vec4(const float a[4]) {
    return vec4(a[0], a[1], a[2], a[3]);
}

vec4_t operator -(const vec4_t& a) {
    return vec4(-a[0], -a[1], -a[2], -a[3]);
}

vec4_t operator +(const vec4_t& a, const vec4_t b) {
    return vec4(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);
}

vec4_t operator -(const vec4_t& a, const vec4_t b) {
    return vec4(a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]);
}

vec4_t operator *(const vec4_t& v, float a) {
    return v * vec4(a);
}

vec4_t operator *(float a, const vec4_t& v) {
    return v * a;
}

vec4_t operator *(const vec4_t& a, const vec4_t& b) {
    return vec4(a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3]);
}

vec4_t operator /(const vec4_t& v, float a) {
    return v * math::recip(a);
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

bool operator ==(const vec4_t& a, const vec4_t& b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

bool operator !=(const vec4_t& a, const vec4_t& b) {
    return !(a == b);
}

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
    return vec4(b[0] < a[0] ? b[0] : a[0],
                b[1] < a[1] ? b[1] : a[1],
                b[2] < a[2] ? b[2] : a[2],
                b[3] < a[3] ? b[3] : a[3]);
}

vec4_t max(const vec4_t& a, const vec4_t& b) {
    return vec4(b[0] > a[0] ? b[0] : a[0],
                b[1] > a[1] ? b[1] : a[1],
                b[2] > a[2] ? b[2] : a[2],
                b[3] > a[3] ? b[3] : a[3]);
}

float sum(const vec4_t& a) {
    return a[0] + a[1] + a[2] + a[3];
}

bool equal(const vec4_t& a, const vec4_t& b, float epsi) {
    return distance(a[0], b[0]) < epsi && distance(a[1], b[1]) < epsi &&
        distance(a[2], b[2]) < epsi && distance(a[3], b[3]) < epsi;
}

MATH_END_NAMESPACE

namespace math {

MATH_INLINE_SPEC float& vec4_t::operator[](unsigned long i) {
    return components[i];
}

MATH_INLINE_SPEC float vec4_t::operator[](unsigned long i) const {
    return components[i];
}

} // namespace math

#endif
