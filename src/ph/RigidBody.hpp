#ifndef PH_RIGID_BODY_HPP
#define PH_RIGID_BODY_HPP

#include "ph/defs.hpp"

#include "math/real.hpp"
#include "math/vec3.hpp"
#include "math/mat3.hpp"

namespace ph {

struct RigidBody {
    math::vec3_t position;
    math::vec3_t velocity;
    math::real invMass;
    math::mat3 invInertia;
};

} // namespace ph

#endif
