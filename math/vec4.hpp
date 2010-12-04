#ifndef VEC4_HPP
#define VEC4_HPP

#include <xmmintrin.h>

#include "Math.hpp"
#include "vec3.hpp"
#include "v4.hpp"

struct vec4 {

    float x, y, z, w;

    vec4() {};

    vec4(float _x, float _y, float _z, float _w)
        : x(_x), y(_y), z(_z), w(_w) {}

    vec4(const vec3& xyz, float _w)
        : x(xyz.x), y(xyz.y), z(xyz.z), w(_w) {}

    vec4(const vec4& a)
        : x(a.x), y(a.y), z(a.z), w(a.w) {}

    vec4& operator =(const vec4& a) {
        x = a.x; y = a.y; z = a.z; w = a.w;
        return *this;
    }

    static vec3 project3(const vec4& a) {
        float rw = unlikely(a.w == 0) ? 1.f : Math::recp(a.w);
        return vec3(a.x * rw, a.y * rw, a.z * rw);
    }

    static vec4 project4(const vec4& a) {
        return unlikely(a.w == 0.f) ? a : vec4(project3(a), 1.f);
    }
        
} __attribute__((aligned(16)));

#endif

