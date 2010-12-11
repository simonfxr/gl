#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "vec3.hpp"
#include "vec4.hpp"
#include "mat4.hpp"

namespace Transform {

namespace {
  
mat4 translate(const vec3& v) {
    return mat4(vec4(1.f, 0.f, 0.f, 0.f),
                vec4(0.f, 1.f, 0.f, 0.f),
                vec4(0.f, 0.f, 1.f, 0.f),
                vec4(v.x, v.y, v.z, 1.f));
}
      
void translate(const vec3& v, mat4& trans, mat4& inv) {
    trans = translate(v);
    inv   = translate(-v);
}

mat4 scale(const vec3& v) {
    return mat4(vec4(v.x, 0.f, 0.f, 0.f),
                vec4(0.f, v.y, 0.f, 0.f),
                vec4(0.f, 0.f, v.z, 0.f),
                vec4(0.f, 0.f, 0.f, 1.f));
}

void scale(const vec3&v, mat4& trans, mat4& inv) {
    trans = scale(v);
    inv   = scale(vec3(Math::recp(v.x), Math::recp(v.y), Math::recp(v.z)));
}

mat4 perspective(float angle_of_view, float aspect_ratio, float z_near, float z_far) {
    float rtan_aov = Math::rtan(angle_of_view);
    return mat4(vec4(rtan_aov, 0.f, 0.f, 0.f),
                vec4(0.f, aspect_ratio * rtan_aov, 0.f, 0.f),
                vec4(0.f, 0.f, (z_far + z_near) / (z_far - z_near), 1.f),
                vec4(0.f, 0.f, -2.f * z_far * z_near / (z_far - z_near), 0.f));
}

} // namespace anon

} // namespace Transform
#endif
