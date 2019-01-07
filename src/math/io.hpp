#ifndef MATH_IO_HPP
#define MATH_IO_HPP

#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

#include "sys/io/Stream.hpp"

sys::io::OutStream &
operator<<(sys::io::OutStream &out, const math::vec3_t &v)
{
    return out << "[" << v[0] << " " << v[1] << " " << v[2] << "]";
}

sys::io::OutStream &
operator<<(sys::io::OutStream &out, const math::vec4_t &v)
{
    return out << "[" << v[0] << " " << v[1] << " " << v[2] << " " << v[3]
               << "]";
}

sys::io::OutStream &
operator<<(sys::io::OutStream &out, const math::mat3_t &m)
{
    const char *sep = "[";

    for (int i = 0; i < 3; ++i) {
        out << sep;
        sep = "; ";
        out << m[i][0] << " " << m[i][1] << " " << m[i][2];
    }

    return out << "]";
}

sys::io::OutStream &
operator<<(sys::io::OutStream &out, const math::mat4_t &m)
{
    const char *sep = "[";

    for (int i = 0; i < 4; ++i) {
        out << sep;
        sep = "; ";
        out << m[i][0] << " " << m[i][1] << " " << m[i][2] << " " << m[i][3];
    }

    return out << "]";
}

#endif
