#include "math/real.hpp"
#include "math/ivec3.hpp"

MATH_BEGIN_NAMESPACE

ivec3_t ivec3(int32 x, int32 y, int32 z) {
    ivec3_t v; v[0] = x; v[1] = y; v[2] = z; return v;
}

ivec3_t ivec3(int32 a) {
    return ivec3(a, a, a);
}

ivec3_t ivec3(const vec3_t& a) {
    return ivec3(int32(a[0]), int32(a[1]), int32(a[2]));
}

ivec3_t ivec3(const int32 a[3]) {
    return ivec3(a[0], a[1], a[2]);
}

ivec3_t operator -(const ivec3_t& a) {
    return ivec3(int32(0)) - a;
}

ivec3_t operator +(const ivec3_t& a, const ivec3_t b) {
    return ivec3(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

ivec3_t operator -(const ivec3_t& a, const ivec3_t b) {
    return ivec3(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}

ivec3_t operator *(const ivec3_t& v, int32 a) {
    return v * ivec3(a);
}

ivec3_t operator *(int32 a, const ivec3_t& v) {
    return v * a;
}

ivec3_t operator *(const ivec3_t& a, const ivec3_t& b) {
    return ivec3(a[0] * b[0], a[1] * b[1], a[2] * b[2]);
}

ivec3_t operator /(const ivec3_t& v, int32 a) {
    return v / ivec3(a);
}

ivec3_t operator /(const ivec3_t& a, const ivec3_t& b) {
    return ivec3(a[0] / b[0], a[1] / b[1], a[2] / b[2]);
}

ivec3_t& operator +=(ivec3_t& v, const ivec3_t& a) {
    return v = v + a;
}

ivec3_t& operator -=(ivec3_t& v, const ivec3_t& a) {
    return v = v - a;
}

ivec3_t& operator *=(ivec3_t& v, int32 a) {
    return v = v * a;
}

ivec3_t& operator *=(ivec3_t& v, const ivec3_t& b) {
    return v = v * b;
}

ivec3_t& operator /=(ivec3_t& v, int32 a) {
    return v = v / a;
}

ivec3_t& operator /=(ivec3_t& v, const ivec3_t& b) {
    return v = v / b;
}

bool operator ==(const ivec3_t& a, const ivec3_t& b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

bool operator !=(const ivec3_t& a, const ivec3_t& b) {
    return !(a == b);
}

int32 dot(const ivec3_t& a, const ivec3_t& b) {
    return sum(a * b);
}

ivec3_t cross(const ivec3_t& a, const ivec3_t& b) {
    return ivec3(a[1] * b[2] - a[2] * b[1],
                a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0]);
}

real length(const ivec3_t& a) {
    return math::sqrt(real(lengthSq(a)));
}

real inverseLength(const ivec3_t& a) {
    return math::rsqrt(real(lengthSq(a)));
}

int32 lengthSq(const ivec3_t& a) {
    return dot(a, a);
}

real distance(const ivec3_t& a, const ivec3_t& b) {
    return length(a - b);
}

real inverseDistance(const ivec3_t& a, const ivec3_t& b) {
    return inverseLength(a - b);
}

int32 distanceSq(const ivec3_t& a, const ivec3_t& b) {
    return lengthSq(a - b);
}

ivec3_t min(const ivec3_t& a, const ivec3_t& b) {
    return ivec3(b[0] < a[0] ? b[0] : a[0],
                b[1] < a[1] ? b[1] : a[1],
                b[2] < a[2] ? b[2] : a[2]);
}

ivec3_t max(const ivec3_t& a, const ivec3_t& b) {
    return ivec3(b[0] > a[0] ? b[0] : a[0],
                b[1] > a[1] ? b[1] : a[1],
                b[2] > a[2] ? b[2] : a[2]);
}

int32 sum(const ivec3_t& a) {
    return a[0] + a[1] + a[2];
}

bool equal(const ivec3_t& a, const ivec3_t& b) {
    return a == b;
}

MATH_END_NAMESPACE

namespace math {

MATH_INLINE_SPEC int32& ivec3_t::operator[](defs::index i) {
    return components[i];
}

MATH_INLINE_SPEC int32 ivec3_t::operator[](defs::index i) const {
    return components[i];
}

} // namespace math
