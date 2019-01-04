#ifndef GLT_FRAME_HPP
#define GLT_FRAME_HPP

#include "glt/conf.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace glt {

/* represents a local right handed coordinate system
 * x_axis is perpendicular to z_axis
 */
struct GLT_API Frame
{
    math::point3_t origin;
    math::direction3_t x_axis;
    math::direction3_t z_axis;

    Frame();

    math::direction3_t localX() const;
    math::direction3_t localY() const;
    math::direction3_t localZ() const;

    void setXY(const math::direction3_t &x, const math::direction3_t &y);

    void setYZ(const math::direction3_t &y, const math::direction3_t &z);

    void setXZ(const math::direction3_t &x, const math::direction3_t &z);

    void lookingAt(const math::point3_t &p);

    void rotateLocal(math::real angleRad, const math::direction3_t &localAxis);

    void rotateWorld(math::real angleRad, const math::direction3_t &worldAxis);

    void translateLocal(const math::vec3_t &v);

    void translateWorld(const math::vec3_t &v);

    void normalize();
};

GLT_API math::vec4_t
transform(const Frame &fr, const math::vec4_t &v);

GLT_API math::point3_t
transformPoint(const Frame &fr, const math::point3_t &p);

GLT_API math::vec3_t
transformVector(const Frame &fr, const math::vec3_t &v);

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
