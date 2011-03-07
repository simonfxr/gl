#ifndef GLT_FRAME_HPP
#define GLT_FRAME_HPP

#include "defs.h"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"
#include "math/mat3/type.hpp"
#include "math/mat4/type.hpp"

namespace glt {

/* represents a local right handed coordinate system
 * x_axis is perpendicular to z_axis
 */ 
struct Frame {

    math::point3_t origin;
    math::direction3_t x_axis;
    math::direction3_t z_axis;

    Frame();

    math::point3_t getOrigin() const;

    math::direction3_t localX() const;
    math::direction3_t localY() const;
    math::direction3_t localZ() const;

    void setXY(const math::direction3_t& x, const math::direction3_t& y);

    void setYZ(const math::direction3_t& y, const math::direction3_t& z);

    void setXZ(const math::direction3_t& x, const math::direction3_t& z);

    void setOrigin(const math::point3_t& p);

    void lookingAt(const math::point3_t& p);

    void rotateLocal(float angleRad, const math::direction3_t& localAxis);

    void rotateWorld(float angleRad, const math::direction3_t& worldAxis);

    void translateLocal(const math::vec3_t& v);

    void translateWorld(const math::vec3_t& v);
};

math::vec4_t transform(const Frame& fr, const math::vec4_t& v);

math::point3_t transformPoint(const Frame& fr, const math::point3_t& p);

math::vec3_t transformVector(const Frame& fr, const math::vec3_t& v);

math::mat3_t rotationLocalToWorld(const Frame& fr);

math::mat3_t rotationWorldToLocal(const Frame& fr);

math::mat4_t transformationLocalToWorld(const Frame& fr);

math::mat4_t transformationWorldToLocal(const Frame& fr);

} // namespace glt

#endif
