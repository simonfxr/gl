#include "glt/Transformations.hpp"

#include "math/real.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

namespace glt {

using namespace math;

mat4_t perspectiveProjection(float radViewAngle, float aspectRatio, float z_near, float z_far) {

    // float f = math::cotan(radViewAngle);

    // float q = math::recip(z_near - z_far);

    // return transpose(mat4(vec4(f / aspectRatio, 0.f, 0.f, 0.f),
    //             vec4(0.f, f, 0.f, 0.f),
    //             vec4(0.f, 0.f, (z_near + z_far) * q, -1.f),
    //                       vec4(0.f, 0.f, 2 * z_near * z_far * q, 0.f)));
    
    float f = cotan(radViewAngle * 0.5f);

    // return mat4(vec4(- f / aspectRatio, 0.f, 0.f, 0.f),
    //             vec4(0.f, f, 0.f, 0.f),
    //             vec4(0.f, 0.f, (z_far + z_near) / (z_far - z_near), 1.f),
    //             vec4(0.f, 0.f, -2.f * z_far * z_near / (z_far - z_near), 0.f));

    return mat4(vec4(f / aspectRatio, 0.f, 0.f, 0.f),
                vec4(0.f, f, 0.f, 0.f),
                vec4(0.f, 0.f, (z_far + z_near) / (z_near - z_far), -1.f),
                vec4(0.f, 0.f, 2.f * z_near * z_far / (z_near - z_far), 0.f));
}

mat3_t rotationMatrix(float theta, const direction3_t& n) {
    float s, c;
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

} // namespace glt
