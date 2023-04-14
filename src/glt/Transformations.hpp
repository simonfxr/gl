#ifndef GLT_TRANSFORMATIONS_HPP
#define GLT_TRANSFORMATIONS_HPP

#include "glt/conf.hpp"
#include "math/fvec.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

namespace glt {

GLT_API math::mat4_t
perspectiveProjection(math::real radViewAngle,
                      math::real aspectRatio,
                      math::real z_near,
                      math::real z_far);

GLT_API math::mat3_t
rotationMatrix(math::real thetaRad, const math::direction3 &axis);

GLT_API math::mat4_t
scaleMatrix(const math::vec3 &scale);

} // namespace glt

#endif
