#ifndef VEC3_HPP
#define VEC3_HPP

#include "Math.hpp"
#include "v4.hpp"

struct vec3 {
    
    float x, y, z;

    static const vec3 zero;

    vec3() {}

    explicit vec3(float a)
        : x(a), y(a), z(a) {}

    vec3(float _x, float _y, float _z)
        : x(_x), y(_y), z(_z) {}

    vec3(const vec3& a)
        : x(a.x), y(a.y), z(a.z) {}

private:

    vec3(v4::v4 v)
        : x(v4::v4a(v)), y(v4::v4b(v)), z(v4::v4c(v)) {}

    static v4::v4 v4(const vec3& v) {
        return v4::make3(v.x, v.y, v.z);
    }
    
public:
    
    static vec3 add(const vec3& a, const vec3& b) {
        return v4::add3(v4(a), v4(b));
    }

    static vec3 sub(const vec3& a, const vec3& b) {
        return v4::sub3(v4(a), v4(b));
    }

    static float dot(const vec3& a, const vec3& b) {
        return v4::dot3(v4(a), v4(b));
    }

    static vec3 cross(const vec3& a, const vec3& b) {
        v4::v4 a1 = v4::make3(a.y, a.z, a.x);
        v4::v4 b1 = v4::make3(b.z, b.x, b.y);
        v4::v4 c1 = v4::mul3(a1, b1);
        v4::v4 a2 = v4::make3(a.z, a.x, a.y);
        v4::v4 b2 = v4::make3(b.y, b.z, b.x);
        v4::v4 c2 = v4::mul3(a2, b2);
        return v4::sub3(c1, c2);
        // return vec3(a.y * b.z - a.z * b.y,
        //             a.z * b.x - a.x * b.z,
        //             a.x * b.y - a.y * b.x);
    }

    static vec3 normalize(const vec3& a) {
        return a.normalize();
    }

    static bool equal(const vec3& a, const vec3& b) {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    static float distSq(const vec3& a, const vec3& b) {
        return (a - b).magSq();
    }

    static float dist(const vec3& a, const vec3& b) {
        return (a - b).mag();
    }

    static float rdist(const vec3& a, const vec3& b) {
        return (a - b).rmag();
    }
    
    static vec3 compMult(const vec3& a, const vec3& b) {
        return v4::mul3(v4(a), v4(b));
    }

    static vec3 reflect(const vec3& a, const vec3& n, float amp = 1.f) {
        return a - n * (2.f * amp * dot(n, a));
    }

    static vec3 max(const vec3& a, const vec3& b) {
        return vec3(b.x > a.x ? b.x : a.x,
                    b.y > a.y ? b.y : a.y,
                    b.z > a.z ? b.z : a.z);
    }

    static vec3 min(const vec3& a, const vec3& b) {
        return vec3(b.x < a.x ? b.x : a.x,
                    b.y < a.y ? b.y : a.y,
                    b.z < a.z ? b.z : a.z);
    }

    vec3 scale(float l) const {
        return compMult(*this, vec3(l));
    }

    vec3 neg() const {
        return v4::neg3(v4(*this));
    }

    float magSq() const {
        return dot(*this, *this);
    }

    float mag() const {
        return Math::sqrt(magSq());
    }

    float rmag() const {
        return Math::rsqrt(magSq());
    }

    vec3 normalize() const {
        return scale(rmag());
    }

    vec3& operator =(const vec3& a) {
        x = a.x; y = a.y; z = a.z;
        return *this;
    }

    bool operator ==(const vec3& a) const {
        return equal(*this, a);
    }

    bool operator !=(const vec3& a) const {
        return !(*this == a);
    }

    vec3 operator +(const vec3& a) const {
        return add(*this, a);
    }

    vec3& operator +=(const vec3& a) {
        return *this = *this + a;
    }

    vec3 operator -(const vec3& a) const {
        return sub(*this, a);
    }

    vec3& operator -=(const vec3& a) {
        return *this = *this - a;
    }

    vec3 operator -() const {
        return neg();
    }

    vec3 operator *(float l) const {
        return scale(l);
    }

    vec3& operator *=(float l) {
        return *this = *this * l;
    }

    vec3 operator /(float l) const {
        return scale(Math::recp(l));
    }
};

namespace {

inline vec3 LOCAL operator *(float l, const vec3& a) {
    return a * l;
}

} // namespace anon

#endif
