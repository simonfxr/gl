#include "glt/Transformations.hpp"

#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/real.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace glt {

using namespace math;

mat4_t
perspectiveProjection(real radViewAngle,
                      real aspectRatio,
                      real z_near,
                      real z_far)
{
    real f = cotan(radViewAngle * real(0.5));

    return mat4(vec4(f / aspectRatio, 0, 0, 0),
                vec4(0, f, 0, 0),
                vec4(0, 0, (z_far + z_near) / (z_near - z_far), -1),
                vec4(0, 0, 2 * z_near * z_far / (z_near - z_far), 0));
}

mat3_t
rotationMatrix(real theta, const direction3_t &n)
{
    real s, c;
    sincos(-theta, s, c);

    return mat3(vec3(n[0] * n[0] * (1 - c) + c,
                     n[0] * n[1] * (1 - c) - n[2] * s,
                     n[0] * n[2] * (1 - c) + n[1] * s),
                vec3(n[1] * n[0] * (1 - c) + n[2] * s,
                     n[1] * n[1] * (1 - c) + c,
                     n[1] * n[2] * (1 - c) - n[0] * s),
                vec3(n[2] * n[0] * (1 - c) - n[1] * s,
                     n[2] * n[1] * (1 - c) + n[0] * s,
                     n[2] * n[2] * (1 - c) + c));
}

math::mat4_t
scaleMatrix(const vec3_t &scale)
{

    return mat4(vec4(scale[0], 0, 0, 0),
                vec4(0, scale[1], 0, 0),
                vec4(0, 0, scale[2], 0),
                vec4(0, 0, 0, 1));
}

} // namespace glt
