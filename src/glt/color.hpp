#ifndef COLOR_HPP
#define COLOR_HPP

#include "defs.h"

#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace glt {

struct color
{
    using byte = uint8_t;

    union
    {
        struct
        {
            byte r, g, b, a;
        };

        uint32_t rgba;
    };

    color() = default;

    constexpr color(byte _r, byte _g, byte _b, byte _a)
      : r(_r), g(_g), b(_b), a(_a)
    {}

    constexpr color(byte _r, byte _g, byte _b) : r(_r), g(_g), b(_b), a(255) {}

    constexpr color(const color &c) : r(c.r), g(c.g), b(c.b), a(c.a) {}

    constexpr explicit color(const math::vec4_t &v)
      : r(byte(255 * v[0]))
      , g(byte(255 * v[1]))
      , b(byte(255 * v[2]))
      , a(byte(255 * v[3]))
    {}

    explicit constexpr color(const math::vec3_t &v)
      : r(byte(255 * v[0])), g(byte(255 * v[1])), b(byte(255 * v[2])), a(255)
    {}

    constexpr color &operator=(const color &c)
    {
        r = c.r;
        g = c.g;
        b = c.b;
        a = c.a;
        return *this;
    }

    constexpr math::vec4_t vec4() const
    {
        using namespace math;
        constexpr math::real Scale = real(1) / real(255);
        return math::vec4(r, g, b, a) * Scale;
    }
};

} // namespace glt

#endif
