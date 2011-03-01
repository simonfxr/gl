#include "glt/Frame.hpp"

#include "math/vec4.hpp"
#include "glt/utils.hpp"

namespace glt {

using namespace math;

#define UNDEFINED() FATAL_ERROR("not yet implemented")

direction3_t Frame::forward() const {
    return z_axis;
}

direction3_t Frame::right() const {
    return x_axis;
}

direction3_t Frame::up() const {
    return cross(x_axis, z_axis);
}

mat3_t Frame::toLocalRotation() const {
    UNDEFINED();
}

mat3_t Frame::fromLocalRotation() const {
    return mat3(x_axis, up(), z_axis);
}

mat4_t Frame::toLocalTransformation() const {
    UNDEFINED();
}

mat4_t Frame::fromLocalTransformation() const {
    return mat4(vec4(x_axis, 0.f), vec4(up(), 0.f),
                vec4(z_axis, 0.f), vec4(origin, 1.f));
}

} // namespace glt
