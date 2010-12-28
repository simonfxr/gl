#ifndef COLOR_HPP
#define COLOR_HPP

#include "defs.h"
#include "math/vec4.hpp"

namespace gltools {

struct color {
    
    byte r, g, b, a;
    
    color() {}

    color(byte _r, byte _g, byte _b, byte _a)
        : r(_r), g(_g), b(_b), a(_a) {}

    color(byte _r, byte _g, byte _b)
        : r(_r), g(_g), b(_b), a(255) {}
    
    color(const color& c)
        : r(c.r), g(c.g), b(c.b), a(c.a) {}

    explicit color(const vec4& v)
        : r(byte(255 * v.x)), g(byte(255 * v.y)),
          b(byte(255 * v.z)), a(byte(255 * v.w)) {}
    
    explicit color(const vec3& v)
        : r(byte(255 * v.x)), g(byte(255 * v.y)),
          b(byte(255 * v.z)), a(255) {}

    color& operator =(const color& c) {
        r = c.r; g = c.g; b = c.b; a = c.a;
        return *this;
    }

    explicit vec4() const {
        static const float Scale = 1.f / 255.f;
        return vec4(r, g, b, a) * Scale;
    }
};

} // gltools

#endif
