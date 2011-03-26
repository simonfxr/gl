#ifndef COLOR_HPP
#define COLOR_HPP

#include "defs.h"

#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace glt {

struct color {
    
    byte r, g, b, a;
    
    color() {}

    color(byte _r, byte _g, byte _b, byte _a)
        : r(_r), g(_g), b(_b), a(_a) {}

    color(byte _r, byte _g, byte _b)
        : r(_r), g(_g), b(_b), a(255) {}
    
    color(const color& c)
        : r(c.r), g(c.g), b(c.b), a(c.a) {}

    explicit color(const math::vec4_t& v)
        : r(byte(255 * v.x)), g(byte(255 * v.y)),
          b(byte(255 * v.z)), a(byte(255 * v.w)) {}
    
    explicit color(const math::vec3_t& v)
        : r(byte(255 * v.x)), g(byte(255 * v.y)),
          b(byte(255 * v.z)), a(255) {}

    color& operator =(const color& c) {
        r = c.r; g = c.g; b = c.b; a = c.a;
        return *this;
    }

    math::vec4_t vec4() const {
        static const float Scale = 1.f / 255.f;
        return math::vec4(r, g, b, a) * Scale;
    }

    uint32 rgba() const {
        return *(uint32 *) &r;
    }
};

} // glt

#endif
