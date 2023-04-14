#include "glt/Frame.hpp"
#include "glt/Transformations.hpp"

#include "math/fvec.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

namespace glt {

using namespace math;

Frame::Frame()
  : origin(vec3(0., 0., 0.)), x_axis(vec3(1., 0., 0.)), z_axis(vec3(0., 0., 1.))
{}

direction3
Frame::localX() const
{
    return x_axis;
}

direction3
Frame::localY() const
{
    return cross(z_axis, x_axis);
}

direction3
Frame::localZ() const
{
    return z_axis;
}

void
Frame::setXY(const direction3 &x, const direction3 &y)
{
    x_axis = x;
    z_axis = cross(x, y);
}

void
Frame::setYZ(const direction3 &y, const direction3 &z)
{
    x_axis = cross(y, z);
    z_axis = z;
}

void
Frame::setXZ(const direction3 &x, const direction3 &z)
{
    x_axis = x;
    z_axis = z;
}

void
Frame::lookingAt(const point3 &p)
{
    auto z = -directionFromTo(origin, p);
    setXZ(math::normalize(cross(localY(), z)), z);
}

void
Frame::rotateLocal(real angleRad, const vec3 &localAxis)
{
    rotateWorld(angleRad, transformVector(*this, localAxis));
}

void
Frame::rotateWorld(real angleRad, const vec3 &worldAxis)
{
    auto rot = rotationMatrix(angleRad, worldAxis);
    x_axis = rot(x_axis);
    z_axis = rot(z_axis);
}

void
Frame::translateLocal(const vec3 &v)
{
    origin += rotationLocalToWorld(*this)(v);
}

void
Frame::translateWorld(const vec3 &v)
{
    origin += v;
}

void
Frame::normalize()
{
    using math::normalize;
    // gram-schmidt orthonormalization
    auto y = normalize(localY());
    auto x = x_axis - projectAlong(x_axis, y);
    auto z = z_axis - projectAlong(z_axis, y) - projectAlong(z_axis, x);
    x_axis = normalize(x);
    z_axis = normalize(z);
}

vec4
transform(const Frame &fr, const vec4 &v)
{
    return transformationLocalToWorld(fr)(v);
}

point3
transformPoint(const Frame &fr, const point3 &p)
{
    return fr.origin + transformVector(fr, p);
}

vec3
transformVector(const Frame &fr, const vec3 &v)
{
    return rotationLocalToWorld(fr)(v);
}

mat3_t
rotationLocalToWorld(const Frame &fr)
{
    return mat3(fr.x_axis, fr.localY(), fr.z_axis);
}

mat3_t
rotationWorldToLocal(const Frame &fr)
{
    return transpose(rotationLocalToWorld(fr));
}

mat4_t
transformationLocalToWorld(const Frame &fr)
{
    return mat4(vec4(fr.x_axis, 0.f),
                vec4(fr.localY(), 0.f),
                vec4(fr.z_axis, 0.f),
                vec4(fr.origin, 1.f));
}

mat4_t
transformationWorldToLocal(const Frame &fr)
{
    auto m = mat4(rotationWorldToLocal(fr));
    m[3] = m(vec4(-fr.origin, 1.f));
    return m;
}

} // namespace glt
