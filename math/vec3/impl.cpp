#if defined(MATH_VEC3_INLINE) || !defined(MATH_INLINE)

#include "math/math.hpp"
#include "math/vec3.hpp"

MATH_BEGIN_NAMESPACE

vec3_t vec3(float x, float y, float z) {
    vec3_t v; v.x = x; v.y = y; v.z = z; return v;
}

vec3_t vec3(float a) {
    return vec3(a, a, a);
}

vec3_t vec3(const vec4_t& a) {
    return vec3(a.x, a.y, a.z);
}

vec3_t vec3(const float a[3]) {
    return vec3(a[0], a[1], a[2]);
}

vec3_t operator -(const vec3_t& a) {
    return vec3(0.f) - a;
}

vec3_t operator +(const vec3_t& a, const vec3_t b) {
    return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

vec3_t operator -(const vec3_t& a, const vec3_t b) {
    return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

vec3_t operator *(const vec3_t& v, float a) {
    return v * vec3(a);
}

vec3_t operator *(float a, const vec3_t& v) {
    return v * a;
}

vec3_t operator *(const vec3_t& a, const vec3_t& b) {
    return vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

vec3_t operator /(const vec3_t& v, float a) {
    return v * math::recp(a);
}

vec3_t operator /(const vec3_t& a, const vec3_t& b) {
    return a * recip(b);
}

vec3_t& operator +=(vec3_t& v, const vec3_t& a) {
    return v = v + a;
}

vec3_t& operator -=(vec3_t& v, const vec3_t& a) {
    return v = v - a;
}

vec3_t& operator *=(vec3_t& v, float a) {
    return v = v * a;
}

vec3_t& operator *=(vec3_t& v, const vec3_t& b) {
    return v = v * b;
}

vec3_t& operator /=(vec3_t& v, float a) {
    return v = v / a;
}

vec3_t& operator /=(vec3_t& v, const vec3_t& b) {
    return v = v / b;
}

bool operator ==(vec3_t& a, vec3_t& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool operator !=(vec3_t& a, vec3_t& b) {
    return !(a == b);
}

float dot(const vec3_t& a, const vec3_t& b) {
    return sum(a * b);
}

vec3_t cross(const vec3_t& a, const vec3_t& b) {
    return vec3(a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x);
}

float length(const vec3_t& a) {
    return math::sqrt(lengthSq(a));
}

float inverseLength(const vec3_t& a) {
    return math::rsqrt(lengthSq(a));
}

float lengthSq(const vec3_t& a) {
    return dot(a, a);
}

direction3_t normalize(const vec3_t& a) {
    return a * inverseLength(a);
}

float distance(const vec3_t& a, const vec3_t& b) {
    return length(a - b);
}

float inverseDistance(const vec3_t& a, const vec3_t& b) {
    return inverseLength(a - b);
}

float distanceSq(const vec3_t& a, const vec3_t& b) {
    return lengthSq(a - b);
}

vec3_t reflect(const vec3_t& a, const normal3_t& n) {
    return reflect(a, n, 1.f);
}

vec3_t reflect(const vec3_t& a, const normal3_t& n, float amp) {
    return a - n * (2.f * amp * dot(n, a));
}

vec3_t min(const vec3_t& a, const vec3_t& b) {
    return vec3(b.x < a.x ? b.x : a.x,
                b.y < a.y ? b.y : a.y,
                b.z < a.z ? b.z : a.z);
}

vec3_t max(const vec3_t& a, const vec3_t& b) {
    return vec3(b.x > a.x ? b.x : a.x,
                b.y > a.y ? b.y : a.y,
                b.z > a.z ? b.z : a.z);
}

float sum(const vec3_t& a) {
    return a.x + a.y + a.z;
}

vec3_t recip(const vec3_t& a) {
    return vec3(recip(a.x), recip(a.y), recip(a.z));
}

vec3_t linearInterpolate(const vec3_t& a, const vec3_t& b, float t) {
    return a + t * (b - a);
}

direction3_t directionFromTo(const point3_t& a, const point3_t& b) {
    return normalize(b - a);
}

float cos(const vec3_t& a, const vec3_t& b) {
    return dot(a, b) / (length(a) * length(b));
}

vec3_t projectAlong(const vec3_t& a, const direction3_t& x) {
    return dot(a, x) * x;
}

bool equal(const vec3_t& a, const vec3_t& b, float epsi) {
    return distance(a[0], b[0]) < epsi && distance(a[1], b[1]) < epsi &&
        distance(a[2], b[2]) < epsi;
}

MATH_END_NAMESPACE

namespace math {

MATH_INLINE_SPEC float& vec3_t::operator[](unsigned long i) {
    return components[i];
}

MATH_INLINE_SPEC float vec3_t::operator[](unsigned long i) const {
    return components[i];
}

} // namespace math

#endif
