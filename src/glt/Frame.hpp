#ifndef GLT_FRAME_HPP
#define GLT_FRAME_HPP

#include "glt/conf.hpp"
#include "math/fvec.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

namespace glt {

/* represents a local right handed coordinate system
 * x_axis is perpendicular to z_axis
 */
struct GLT_API Frame
{
    math::point3 origin;
    math::direction3 x_axis;
    math::direction3 z_axis;

    Frame();

    math::direction3 localX() const;
    math::direction3 localY() const;
    math::direction3 localZ() const;

    void setXY(const math::direction3 &x, const math::direction3 &y);

    void setYZ(const math::direction3 &y, const math::direction3 &z);

    void setXZ(const math::direction3 &x, const math::direction3 &z);

    void lookingAt(const math::point3 &p);

    void rotateLocal(math::real angleRad, const math::direction3 &localAxis);

    void rotateWorld(math::real angleRad, const math::direction3 &worldAxis);

    void translateLocal(const math::vec3 &v);

    void translateWorld(const math::vec3 &v);

    void normalize();
};

GLT_API math::vec4
transform(const Frame &fr, const math::vec4 &v);

GLT_API math::point3
transformPoint(const Frame &fr, const math::point3 &p);

GLT_API math::vec3
transformVector(const Frame &fr, const math::vec3 &v);

GLT_API math::mat3_t
rotationLocalToWorld(const Frame &fr);

GLT_API math::mat3_t
rotationWorldToLocal(const Frame &fr);

GLT_API math::mat4_t
transformationLocalToWorld(const Frame &fr);

GLT_API math::mat4_t
transformationWorldToLocal(const Frame &fr);

} // namespace glt

#endif
