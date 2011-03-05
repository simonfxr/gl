#include "glt/Frame.hpp"

#include "glt/utils.hpp"

#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/math.hpp"

#include <GLFrame.h>

#include <iostream>

namespace {

using namespace math;

std::ostream& operator <<(std::ostream &out, const vec3_t& v) {
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

std::ostream& operator <<(std::ostream &out, const vec4_t& v) {
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}

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
    fr.frame = ref;
}

void validate(const glt::Frame& fr);

void update(glt::Frame& fr) {
    vec3_t up = cross(fr.z_axis, fr.x_axis);
    fr.frame.SetUpVector(&up[0]);
    fr.frame.SetForwardVector(&fr.z_axis[0]);
    validate(fr);
}

bool vecEqual(const vec3_t& a, const float *b) {
    const float epsi = 1e-4f;
    return distance(a[0], b[0]) < epsi && distance(a[1], b[1]) < epsi &&
        distance(a[2], b[2]) < epsi;
}

bool vecEqual(const vec4_t& a, const float *b) {
    const float epsi = 1e-4f;
    return distance(a[0], b[0]) < epsi && distance(a[1], b[1]) < epsi &&
        distance(a[2], b[2]) < epsi && distance(a[3], b[3]) < epsi;
}

bool check(const glt::Frame& fr) {
    M3DVector3f v;
    fr.frame.GetXAxis(v);
    if (!vecEqual(fr.localX(), v)) return false;
    fr.frame.GetYAxis(v);
    if (!vecEqual(fr.localY(), v)) return false;
    fr.frame.GetZAxis(v);
    if (!vecEqual(fr.localZ(), v)) return false;
    fr.frame.GetOrigin(v);
    if (!vecEqual(fr.origin, v)) return false;
    return true;
}

void validate(const glt::Frame& fr) {
    static bool validating = false;
    if (validating) return;
    validating = true;
    if (!check(fr))
        FATAL_ERROR("frame validation failed");
    validating = false;
}

} // namespace anon

namespace glt {

using namespace math;

#define UNDEFINED() FATAL_ERROR("not yet implemented")

Frame::Frame() {
    x_axis = vec3(-1.f, 0.f, 0.f);
    z_axis = vec3(0.f, 0.f, -1.f);
    origin = vec3(0.f, 0.f, 0.f);
    update(*this);
}

point3_t Frame::getOrigin() const {
    validate(*this);
    return origin;
}

direction3_t Frame::localX() const {
    validate(*this);
    return x_axis;
}

direction3_t Frame::localY() const {
    validate(*this);
    return cross(z_axis, x_axis);
}

direction3_t Frame::localZ() const {
    validate(*this);
    return z_axis;
}

void Frame::setXY(const direction3_t& x, const direction3_t& y) {
    validate(*this);
    
    x_axis = x;
    z_axis = cross(x, y);

    update(*this);

}

void Frame::setYZ(const direction3_t& y, const direction3_t& z) {
    validate(*this);
    
    x_axis = cross(y, z);
    z_axis = z;

    update(*this);
}

void Frame::setXZ(const direction3_t& x, const direction3_t& z) {
    validate(*this);

    x_axis = x;
    z_axis = z;

    update(*this);
}

void Frame::setOrigin(const point3_t& p) {
    validate(*this);
    
    origin = p;
    point3_t tmp = p;
    frame.SetOrigin(&tmp[0]);

}

mat4_t Frame::cameraMatrix() const {

    validate(*this);
    
    direction3_t y = localY();
    direction3_t z = -localZ();
    direction3_t x = -localX();

    mat4_t trans = transpose(mat4(vec4(x, 0.f), vec4(y, 0.f), vec4(z, 0.f),
                                  vec4(0.f, 0.f, 0.f, 1.f)));

    trans = trans * mat4(vec4(1.f, 0.f, 0.f, 0.f),
                         vec4(0.f, 1.f, 0.f, 0.f),
                         vec4(0.f, 0.f, 1.f, 0.f),
                         vec4(-origin, 1.f));

    M3DMatrix44f trans2;
    frame.GetCameraMatrix(trans2);

    if (!vecEqual(trans[0], trans2 + 0) || !vecEqual(trans[1], trans2 + 4) ||
        !vecEqual(trans[2], trans2 + 8) || !vecEqual(trans[3], trans2 + 12)) {

        vec3_t fx, fy, fz, O;
        frame.GetXAxis(&fx[0]);
        frame.GetYAxis(&fy[0]);
        frame.GetZAxis(&fz[0]);
        frame.GetOrigin(&O[0]);

        std::cerr << "x: " << localX() << std::endl
                  << "y: " << localY() << std::endl
                  << "z: " << localZ() << std::endl
                  << "O: " << origin << std::endl

                  << "frame:" << std::endl
                  << "x: " << fx << std::endl
                  << "y: " << fy << std::endl
                  << "z: " << fz << std::endl
                  << "O: " << O << std::endl
            
                  << "matrix:" << std::endl
                  << trans[0] << std::endl
                  << trans[1] << std::endl
                  << trans[2] << std::endl
                  << trans[3] << std::endl;
        
        FATAL_ERROR("frame: camera matrix validation failed");
    }

    return trans;
}

void Frame::rotateLocal(float angleRad, const vec3_t& localAxis) {
    validate(*this);
    GLFrame fr = asGLFrame(*this);
    fr.RotateLocal(angleRad, localAxis.x, localAxis.y, localAxis.z);
    setFrame(*this, fr);
    validate(*this);
}

void Frame::rotateWorld(float angleRad, const vec3_t& worldAxis) {
    validate(*this);
    GLFrame fr = asGLFrame(*this);
    fr.RotateWorld(angleRad, worldAxis.x, worldAxis.y, worldAxis.z);
    setFrame(*this, fr);
    validate(*this);
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
    UNUSED(fr);
    UNDEFINED();
}

mat4_t transformationLocalToWorld(const Frame& fr) {
    return mat4(vec4(fr.x_axis, 0.f), vec4(fr.localY(), 0.f),
                vec4(fr.z_axis, 0.f), vec4(fr.origin, 1.f));
}

mat4_t transformationWorldToLocal(const Frame& fr) {
    UNUSED(fr);
    UNDEFINED();
}

} // namespace glt
