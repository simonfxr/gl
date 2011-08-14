#include "math/vec3/defns.hpp"
#include "math/real.hpp"

MATH_BEGIN_NAMESPACE

vec3_t vec3(real x, real y, real z) {
    vec3_t v; v[0] = x; v[1] = y; v[2] = z; return v;
}

vec3_t vec3(real a) {
    return vec3(a, a, a);
}

vec3_t vec3(const ivec3_t& a) {
    return vec3(real(a[0]), real(a[1]), real(a[2]));
}

vec3_t vec3(const vec4_t& a) {
    return vec3(a[0], a[1], a[2]);
}

vec3_t vec3(const real a[3]) {
    return vec3(a[0], a[1], a[2]);
}

vec3_t operator -(const vec3_t& a) {
    return vec3(0.f) - a;
}

vec3_t operator +(const vec3_t& a, const vec3_t b) {
    return vec3(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

vec3_t operator -(const vec3_t& a, const vec3_t b) {
    return vec3(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}

vec3_t operator *(const vec3_t& v, real a) {
    return v * vec3(a);
}

vec3_t operator *(real a, const vec3_t& v) {
    return v * a;
}

vec3_t operator *(const vec3_t& a, const vec3_t& b) {
    return vec3(a[0] * b[0], a[1] * b[1], a[2] * b[2]);
}

vec3_t operator /(const vec3_t& v, real a) {
    return v * math::recip(a);
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

vec3_t& operator *=(vec3_t& v, real a) {
    return v = v * a;
}

vec3_t& operator *=(vec3_t& v, const vec3_t& b) {
    return v = v * b;
}

vec3_t& operator /=(vec3_t& v, real a) {
    return v = v / a;
}

vec3_t& operator /=(vec3_t& v, const vec3_t& b) {
    return v = v / b;
}

bool operator ==(const vec3_t& a, const vec3_t& b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

bool operator !=(const vec3_t& a, const vec3_t& b) {
    return !(a == b);
}

real dot(const vec3_t& a, const vec3_t& b) {
    return sum(a * b);
}

vec3_t cross(const vec3_t& a, const vec3_t& b) {
    return vec3(a[1] * b[2] - a[2] * b[1],
                a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0]);
}

real length(const vec3_t& a) {
    return math::sqrt(lengthSq(a));
}

real inverseLength(const vec3_t& a) {
    return math::rsqrt(lengthSq(a));
}

real lengthSq(const vec3_t& a) {
    return dot(a, a);
}

direction3_t normalize(const vec3_t& a) {
    return a * inverseLength(a);
}

real distance(const vec3_t& a, const vec3_t& b) {
    return length(a - b);
}

real inverseDistance(const vec3_t& a, const vec3_t& b) {
    return inverseLength(a - b);
}

real distanceSq(const vec3_t& a, const vec3_t& b) {
    return lengthSq(a - b);
}

vec3_t reflect(const vec3_t& a, const normal3_t& n) {
    return reflect(a, n, 1.f);
}

vec3_t reflect(const vec3_t& a, const normal3_t& n, real amp) {
    return a - n * (2.f * amp * dot(n, a));
}

vec3_t min(const vec3_t& a, const vec3_t& b) {
    return vec3(b[0] < a[0] ? b[0] : a[0],
                b[1] < a[1] ? b[1] : a[1],
                b[2] < a[2] ? b[2] : a[2]);
}

vec3_t max(const vec3_t& a, const vec3_t& b) {
    return vec3(b[0] > a[0] ? b[0] : a[0],
                b[1] > a[1] ? b[1] : a[1],
                b[2] > a[2] ? b[2] : a[2]);
}

real sum(const vec3_t& a) {
    return a[0] + a[1] + a[2];
}

vec3_t recip(const vec3_t& a) {
    return vec3(recip(a[0]), recip(a[1]), recip(a[2]));
}

vec3_t abs(const vec3_t& a) {
    return vec3(abs(a[0]), abs(a[1]), abs(a[2]));
}

vec3_t linearInterpolate(const vec3_t& a, const vec3_t& b, real t) {
    return a + t * (b - a);
}

direction3_t directionFromTo(const point3_t& a, const point3_t& b) {
    return normalize(b - a);
}

real cos(const vec3_t& a, const vec3_t& b) {
    return dot(a, b) / (length(a) * length(b));
}

vec3_t projectAlong(const vec3_t& a, const vec3_t& x) {
    return (dot(a, x) / lengthSq(x)) * x;
}

bool equal(const vec3_t& a, const vec3_t& b, real epsi) {
    return distance(a[0], b[0]) < epsi &&
           distance(a[1], b[1]) < epsi &&
           distance(a[2], b[2]) < epsi;
}

MATH_END_NAMESPACE

namespace math {

MATH_INLINE_SPEC real& vec3_t::operator[](index_t i) {
    return components[i];
}

MATH_INLINE_SPEC real vec3_t::operator[](index_t i) const {
    return components[i];
}

} // namespace math
