#ifndef GLT_GEOMETRIC_HPP
#define GLT_GEOMETRIC_HPP

#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

#include "defs.hpp"

namespace glt {

constexpr math::mat4_t
translateTransform(math::vec3_t point)
{
    auto M = math::mat4();
    M[3] = math::vec4(point, math::real(1));
    return M;
}

constexpr math::mat4_t
scaleTransform(math::vec3_t dim)
{
    using namespace math;
    return mat4(mat3(vec3(dim[0], real(0), real(0)),
                     vec3(real(0), dim[1], real(0)),
                     vec3(real(0), real(0), dim[2])));
}

constexpr math::mat4_t
scaleTransform(math::real dim)
{
    return scaleTransform(math::vec3(dim));
}

} // namespace glt

#endif
