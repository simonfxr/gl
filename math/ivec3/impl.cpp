#if defined(MATH_IVEC3_INLINE) || !defined(MATH_INLINE)

#include "math/math.hpp"
#include "math/ivec3.hpp"

MATH_BEGIN_NAMESPACE

ivec3_t ivec3(int32 x, int32 y, int32 z) {
    ivec3_t v; v.x = x; v.y = y; v.z = z; return v;
}

ivec3_t ivec3(int32 a) {
    return ivec3(a, a, a);
}

ivec3_t ivec3(const vec3_t& a) {
    return ivec3(int32(a.x), int32(a.y), int32(a.z));
}

ivec3_t ivec3(const int32 a[3]) {
    return ivec3(a[0], a[1], a[2]);
}

ivec3_t operator -(const ivec3_t& a) {
    return ivec3(0.f) - a;
}

ivec3_t operator +(const ivec3_t& a, const ivec3_t b) {
    return ivec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

ivec3_t operator -(const ivec3_t& a, const ivec3_t b) {
    return ivec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

ivec3_t operator *(const ivec3_t& v, int32 a) {
    return v * ivec3(a);
}

ivec3_t operator *(int32 a, const ivec3_t& v) {
    return v * a;
}

ivec3_t operator *(const ivec3_t& a, const ivec3_t& b) {
    return ivec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

ivec3_t operator /(const ivec3_t& v, int32 a) {
    return v / ivec3(a);
}

ivec3_t operator /(const ivec3_t& a, const ivec3_t& b) {
    return ivec3(a.x / b.x, a.y / b.y, a.z / b.z);
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
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool operator !=(const ivec3_t& a, const ivec3_t& b) {
    return !(a == b);
}

int32 dot(const ivec3_t& a, const ivec3_t& b) {
    return sum(a * b);
}

ivec3_t cross(const ivec3_t& a, const ivec3_t& b) {
    return ivec3(a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x);
}

float length(const ivec3_t& a) {
    return math::sqrt(lengthSq(a));
}

float inverseLength(const ivec3_t& a) {
    return math::rsqrt(lengthSq(a));
}

int32 lengthSq(const ivec3_t& a) {
    return dot(a, a);
}

float distance(const ivec3_t& a, const ivec3_t& b) {
    return length(a - b);
}

float inverseDistance(const ivec3_t& a, const ivec3_t& b) {
    return inverseLength(a - b);
}

int32 distanceSq(const ivec3_t& a, const ivec3_t& b) {
    return lengthSq(a - b);
}

ivec3_t min(const ivec3_t& a, const ivec3_t& b) {
    return ivec3(b.x < a.x ? b.x : a.x,
                b.y < a.y ? b.y : a.y,
                b.z < a.z ? b.z : a.z);
}

ivec3_t max(const ivec3_t& a, const ivec3_t& b) {
    return ivec3(b.x > a.x ? b.x : a.x,
                b.y > a.y ? b.y : a.y,
                b.z > a.z ? b.z : a.z);
}

int32 sum(const ivec3_t& a) {
    return a.x + a.y + a.z;
}

bool equal(const ivec3_t& a, const ivec3_t& b) {
    return a == b;
}

MATH_END_NAMESPACE

namespace math {

MATH_INLINE_SPEC int32& ivec3_t::operator[](unsigned long i) {
    return components[i];
}

MATH_INLINE_SPEC int32 ivec3_t::operator[](unsigned long i) const {
    return components[i];
}

} // namespace math

#endif
