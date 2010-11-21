#ifndef VEC3_HPP
#define VEC3_HPP

#include "Math.hpp"

struct vec3 {
    
    float x, y, z;

    vec3() {}

    vec3(float _x, float _y, float _z)
        : x(_x), y(_y), z(_z) {}

    vec3(const vec3& a)
        : x(a.x), y(a.y), z(a.z) {}
    
    static vec3 add(const vec3& a, const vec3& b) {
        return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    static vec3 sub(const vec3& a, const vec3& b) {
        return add(a, b.neg());
    }

    static float dot(const vec3& a, const vec3& b) {
        return a.x * b.x + a.y * b.y +  a.z * b.z;
    }

    static vec3 cross(const vec3& a, const vec3& b) {
        return vec3(a.y * b.z - a.z * b.y,
                    a.z * b.x - a.x * b.z,
                    a.x * b.y - a.y * b.x);
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

    vec3 scale(float l) const {
        return vec3(x * l, y * l, z * l);
    }

    vec3 neg() const {
        return vec3(-x, -y, -z);
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

    vec3 normal() const {
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

    vec3 operator -(const vec3& a) const {
        return sub(*this, a);
    }

    vec3 operator -() const {
        return neg();
    }

    vec3 operator *(float l) const {
        return scale(l);
    }

    vec3 operator /(float l) const {
        return scale(Math::recp(l));
    }
};

namespace {
    inline vec3 operator *(float l, const vec3& a) {
        return a * l;
    }
}

#endif
