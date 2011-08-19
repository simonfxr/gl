#ifndef GLT_TRANSFORMATIONS_HPP
#define GLT_TRANSFORMATIONS_HPP

#include "math/mat3/type.hpp"
#include "math/mat4/type.hpp"

namespace glt {

// also inverts x and z axis!
math::mat4_t perspectiveProjection(float radViewAngle, float aspectRatio, float z_near, float z_far);

math::mat3_t rotationMatrix(float radAngle, const math::direction3_t& axis);

math::mat4_t scaleMatrix(const math::vec3_t& scale);

} // namespace glt

#endif
