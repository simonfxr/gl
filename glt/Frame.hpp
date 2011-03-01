#ifndef GLT_FRAME_HPP
#define GLT_FRAME_HPP

#include "defs.h"
#include "math/vec3.hpp"
#include "math/mat4.hpp"
#include "math/mat3.hpp"

namespace glt {

/* represents a local right handed coordinate system
 * x_axis is perpendicular to the z_axis
 */ 
struct Frame {

    math::point3_t origin;
    math::direction3_t x_axis;
    math::direction3_t z_axis;

    math::direction3_t forward() const;
    math::direction3_t right() const;
    math::direction3_t up() const;

    math::mat3_t toLocalRotation() const;
    math::mat3_t fromLocalRotation() const;

    math::mat4_t toLocalTransformation() const;
    math::mat4_t fromLocalTransformation() const;
};

} // namespace glt

#endif
