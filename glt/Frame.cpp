#include "glt/Frame.hpp"

#include "glt/utils.hpp"

#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

namespace {

using namespace math;

GLFrame asGLFrame(const glt::Frame& ref) {
    GLFrame fr;

    math::point3_t pos = ref.origin;
    fr.SetOrigin(pos.x, pos.y, pos.z);
    math::direction3_t fw = ref.localZ();
    fr.SetForwardVector(fw.x, fw.y, fw.z);
    math::direction3_t up = ref.localY();
    fr.SetUpVector(up.x, up.y, up.z);
    return fr;
}

void setFrame(glt::Frame& fr, GLFrame& ref) {
    ref.GetOrigin(&fr.origin[0]);
    ref.GetXAxis(&fr.x_axis[0]);
    ref.GetZAxis(&fr.z_axis[0]);
}

} // namespace anon

namespace glt {

using namespace math;

#define UNDEFINED() FATAL_ERROR("not yet implemented")

Frame::Frame() {
    x_axis = vec3(-1.f, 0.f, 0.f);
    z_axis = vec3(0.f, 0.f, -1.f);
    origin = vec3(0.f, 0.f, 0.f);
}

point3_t Frame::getOrigin() const {
    return origin;
}

direction3_t Frame::localX() const {
    return x_axis;
}

direction3_t Frame::localY() const {
    return cross(z_axis, x_axis);
}

direction3_t Frame::localZ() const {
    return z_axis;
}

void Frame::setXY(const direction3_t& x, const direction3_t& y) {
    x_axis = x;
    z_axis = cross(x, y);
}

void Frame::setYZ(const direction3_t& y, const direction3_t& z) {
    x_axis = cross(y, z);
    z_axis = z;
}

void Frame::setXZ(const direction3_t& x, const direction3_t& z) {
    x_axis = x;
    z_axis = z;
}

void Frame::setOrigin(const point3_t& p) {
    origin = p;
}

mat4_t Frame::cameraMatrix() const {
    direction3_t y = localY();
    direction3_t z = -localZ();
    direction3_t x = -localX();

    mat4_t m = mat4(transpose(mat3(x, y, z)));
    m[3] = m * vec4(-origin, 1.f);

    return m;
}

void Frame::rotateLocal(float angleRad, const vec3_t& localAxis) {
    GLFrame fr = asGLFrame(*this);
    fr.RotateLocal(angleRad, localAxis.x, localAxis.y, localAxis.z);
    setFrame(*this, fr);
}

void Frame::rotateWorld(float angleRad, const vec3_t& worldAxis) {
    GLFrame fr = asGLFrame(*this);
    fr.RotateWorld(angleRad, worldAxis.x, worldAxis.y, worldAxis.z);
    setFrame(*this, fr);
}

void Frame::translateLocal(const vec3_t& v) {
    setOrigin(origin + rotationLocalToWorld(*this) * v);
}

void Frame::translateWorld(const vec3_t& v) {
    setOrigin(origin + v);
}

vec4_t transform(const Frame& fr, const vec4_t& v) {
    return transformationLocalToWorld(fr) * v;
}

point3_t transformPoint(const Frame& fr, const point3_t& p) {
    return fr.origin + transformVector(fr, p);
}

vec3_t transformVector(const Frame& fr, const vec3_t& v) {
    return rotationLocalToWorld(fr) * v;
}

mat3_t rotationLocalToWorld(const Frame& fr) {
    return mat3(fr.x_axis, fr.localY(), fr.z_axis);
}

mat3_t rotationWorldToLocal(const Frame& fr) {
    return transpose(rotationLocalToWorld(fr));
}

mat4_t transformationLocalToWorld(const Frame& fr) {
    return mat4(vec4(fr.x_axis, 0.f), vec4(fr.localY(), 0.f),
                vec4(fr.z_axis, 0.f), vec4(fr.origin, 1.f));
}

mat4_t transformationWorldToLocal(const Frame& fr) {
    mat4_t m = mat4(rotationWorldToLocal(fr));
    m[3] = m * vec4(-fr.origin, 1.f);
    return m;
}

} // namespace glt
