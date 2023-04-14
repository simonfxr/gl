#ifndef COLOR_HPP
#define COLOR_HPP

#include "defs.h"

#include "math/fvec.hpp"

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

    constexpr color() noexcept = default;

    constexpr color(byte _r, byte _g, byte _b, byte _a)
      : r(_r), g(_g), b(_b), a(_a)
    {}

    constexpr color(byte _r, byte _g, byte _b) : r(_r), g(_g), b(_b), a(255) {}

    constexpr color(const color &c) noexcept = default;
    constexpr color(color &&c) noexcept = default;

    constexpr explicit color(math::vec<byte, 4> rgba)
      : color(rgba[0], rgba[1], rgba[2], rgba[3])
    {}

    constexpr explicit color(math::vec<byte, 3> rgb)
      : color(rgb[0], rgb[1], rgb[2], 255)
    {}

    constexpr explicit color(const math::vec4 &v)
      : r(byte(255 * v[0]))
      , g(byte(255 * v[1]))
      , b(byte(255 * v[2]))
      , a(byte(255 * v[3]))
    {}

    explicit constexpr color(const math::vec3 &v)
      : r(byte(255 * v[0])), g(byte(255 * v[1])), b(byte(255 * v[2])), a(255)
    {}

    constexpr color &operator=(const color &c) noexcept = default;
    constexpr color &operator=(color &&c) noexcept = default;

    [[nodiscard]] constexpr math::vec4 vec4() const
    {
        using namespace math;
        constexpr math::real Scale = 1_r / 255_r;
        return math::vec4(r, g, b, a) * Scale;
    }

    [[nodiscard]] constexpr math::vec<uint8_t, 4> u8vec4() const
    {
        return { r, g, b, a };
    }
};

} // namespace glt

#endif
