#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat4.hpp"

namespace math {

namespace Transform {

namespace {
  
mat4 LOCAL translate(const vec3_t& v) {
    return mat4(vec4(1.f, 0.f, 0.f, 0.f),
                vec4(0.f, 1.f, 0.f, 0.f),
                vec4(0.f, 0.f, 1.f, 0.f),
                vec4(v.x, v.y, v.z, 1.f));
}
      
void LOCAL translate(const vec3_t& v, mat4& trans, mat4& inv) {
    trans = translate(v);
    inv   = translate(-v);
}

mat4 LOCAL scale(const vec3_t& v) {
    return mat4(vec4(v.x, 0.f, 0.f, 0.f),
                vec4(0.f, v.y, 0.f, 0.f),
                vec4(0.f, 0.f, v.z, 0.f),
                vec4(0.f, 0.f, 0.f, 1.f));
}

void LOCAL scale(const vec3_t&v, mat4& trans, mat4& inv) {
    trans = scale(v);
    inv   = scale(vec3(recp(v.x), recp(v.y), recp(v.z)));
}

mat4 LOCAL perspective(float angle_of_view, float aspect_ratio, float z_near, float z_far) {
    float rtan_aov = rtan(angle_of_view);
    return mat4(vec4(rtan_aov, 0.f, 0.f, 0.f),
                vec4(0.f, aspect_ratio * rtan_aov, 0.f, 0.f),
                vec4(0.f, 0.f, (z_far + z_near) / (z_far - z_near), 1.f),
                vec4(0.f, 0.f, -2.f * z_far * z_near / (z_far - z_near), 0.f));
}

} // namespace anon

} // namespace Transform

} // namespace math
#endif
