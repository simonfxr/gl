#if defined(MATH_VEC2_INLINE) || !defined(MATH_INLINE)

#include "math/math.hpp"
#include "math/vec2.hpp"

MATH_BEGIN_NAMESPACE

vec2_t vec2(float x, float y) {
    vec2_t v; v.x = x; v.y = y; return v;
}

vec2_t vec2(float a) {
    return vec2(a, a);
}

vec2_t vec2(const vec3_t& a) {
    return vec2(a.x, a.y);
}
vec2_t vec2(const vec4_t& a) {
    return vec2(a.x, a.y);
}

vec2_t vec2(const float a[2]) {
    return vec2(a[0], a[1]);
}

vec2_t operator -(const vec2_t& a) {
    return vec2(0.f) - a;
}

vec2_t operator +(const vec2_t& a, const vec2_t b) {
    return vec2(a.x + b.x, a.y + b.y);
}

vec2_t operator -(const vec2_t& a, const vec2_t b) {
    return vec2(a.x - b.x, a.y - b.y);
}

vec2_t operator *(const vec2_t& v, float a) {
    return v * vec2(a);
}

vec2_t operator *(float a, const vec2_t& v) {
    return v * a;
}

vec2_t operator *(const vec2_t& a, const vec2_t& b) {
    return vec2(a.x * b.x, a.y * b.y);
}

vec2_t operator /(const vec2_t& v, float a) {
    return v * math::recp(a);
}

vec2_t& operator +=(vec2_t& v, const vec2_t& a) {
    return v = v + a;
}

vec2_t& operator -=(vec2_t& v, const vec2_t& a) {
    return v = v - a;
}

vec2_t& operator *=(vec2_t& v, float a) {
    return v = v * a;
}

vec2_t& operator *=(vec2_t& v, const vec2_t& b) {
    return v = v * b;
}

vec2_t& operator /=(vec2_t& v, float a) {
    return v = v / a;
}

bool operator ==(vec2_t& a, vec2_t& b) {
    return a.x == b.x && a.y == b.y;
}

bool operator !=(vec2_t& a, vec2_t& b) {
    return !(a == b);
}

float dot(const vec2_t& a, const vec2_t& b) {
    return sum(a * b);
}

float length(const vec2_t& a) {
    return math::sqrt(lengthSq(a));
}

float inverseLength(const vec2_t& a) {
    return math::rsqrt(lengthSq(a));
}

float lengthSq(const vec2_t& a) {
    return dot(a, a);
}

direction2_t normalize(const vec2_t& a) {
    return a * inverseLength(a);
}

float distance(const vec2_t& a, const vec2_t& b) {
    return length(a - b);
}

float inverseDistance(const vec2_t& a, const vec2_t& b) {
    return inverseLength(a - b);
}

float distanceSq(const vec2_t& a, const vec2_t& b) {
    return lengthSq(a - b);
}

vec2_t reflect(const vec2_t& a, const normal2_t& n) {
    return reflect(a, n, 1.f);
}

vec2_t reflect(const vec2_t& a, const normal2_t& n, float amp) {
    return a - n * (2.f * amp * dot(n, a));
}

vec2_t min(const vec2_t& a, const vec2_t& b) {
    return vec2(b.x < a.x ? b.x : a.x,
                b.y < a.y ? b.y : a.y);
}

vec2_t max(const vec2_t& a, const vec2_t& b) {
    return vec2(b.x > a.x ? b.x : a.x,
                b.y > a.y ? b.y : a.y);
}

float sum(const vec2_t& a) {
    return a.x + a.y;
}

vec2_t linearInterpolate(const vec2_t& a, const vec2_t& b, float t) {
    return a + t * (b - a);
}

direction2_t directionFromTo(const point2_t& a, const point2_t& b) {
    return normalize(b - a);
}

float cos(const vec2_t& a, const vec2_t& b) {
    return dot(a, b) / (length(a) * length(b));
}

vec2_t projectAlong(const vec2_t& a, const direction2_t& x) {
    return dot(a, x) * x;
}

bool equal(const vec2_t& a, const vec2_t& b, float epsi) {
    return distance(a[0], b[0]) < epsi && distance(a[1], b[1]) < epsi;
}

MATH_END_NAMESPACE

namespace math {

MATH_INLINE_SPEC float& vec2_t::operator[](unsigned long i) {
    return components[i];
}

MATH_INLINE_SPEC float vec2_t::operator[](unsigned long i) const {
    return components[i];
}

} // namespace math

#endif
