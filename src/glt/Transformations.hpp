#ifndef GLT_TRANSFORMATIONS_HPP
#define GLT_TRANSFORMATIONS_HPP

#include "glt/conf.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/vec3.hpp"

namespace glt {

GLT_API math::mat4_t
perspectiveProjection(math::real radViewAngle,
                      math::real aspectRatio,
                      math::real z_near,
                      math::real z_far);

GLT_API math::mat3_t
rotationMatrix(math::real radAngle, const math::direction3_t &axis);

GLT_API math::mat4_t
scaleMatrix(const math::vec3_t &scale);

} // namespace glt

#endif
