#ifndef GLT_GEOMETRIC_HPP
#define GLT_GEOMETRIC_HPP

#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

#include "defs.hpp"

namespace glt {

using namespace math;

constexpr mat4_t
translateTransform(vec3_t point)
{
    mat4_t M = mat4();
    M[3] = vec4(point, real(1));
    return M;
}

constexpr mat4_t
scaleTransform(vec3_t dim)
{
    return mat4(mat3(vec3(dim[0], real(0), real(0)),
                     vec3(real(0), dim[1], real(0)),
                     vec3(real(0), real(0), dim[2])));
}

constexpr mat4_t
scaleTransform(real dim)
{
    return scaleTransform(vec3(dim));
}

} // namespace glt

#endif
