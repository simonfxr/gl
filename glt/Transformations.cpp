#include "glt/Transformations.hpp"

#include "math/math.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

namespace glt {

using namespace math;

mat4_t perspectiveProjection(float radViewAngle, float aspectRatio, float z_near, float z_far) {
    float recp_tan_fov = rtan(radViewAngle);
    float recp_aspect = recip(aspectRatio);

    return mat4(vec4(-recp_aspect * recp_tan_fov, 0.f, 0.f, 0.f),
                vec4(0.f, recp_tan_fov, 0.f, 0.f),
                vec4(0.f, 0.f, (z_far + z_near) / (z_far - z_near), 1.f),
                vec4(0.f, 0.f, -2.f * z_far * z_near / (z_far - z_near), 0.f));
}

mat3_t rotationMatrix(float theta, const direction3_t& n) {
    float s, c;
    sincos(-theta, s, c);

    return mat3(vec3(n.x * n.x * (1 - c) + c,
                     n.x * n.y * (1 - c) - n.z * s,
                     n.x * n.z * (1 - c) + n.y * s),
                vec3(n.y * n.x * (1 - c) + n.z * s,
                     n.y * n.y * (1 - c) + c,
                     n.y * n.z * (1 - c) - n.x * s),
                vec3(n.z * n.x * (1 - c) - n.y * s,
                     n.z * n.y * (1 - c) + n.x * s,
                     n.z * n.z * (1 - c) + c));
}

} // namespace glt
