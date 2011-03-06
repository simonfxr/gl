#ifndef PLANE_HPP
#define PLANE_HPP

#include "math/defs.hpp"
#include "math/vec3.hpp"

namespace math {

// every point x on the plane solves dot(normal, x - dist * normal) = 0
struct plane3_t {
    direction3_t normal;
    float dist; // distance to origin
};

plane3_t plane(const direction3_t& normal, float dist) PURE_FUNC;

// plane through a triangle: if the triangle is viewed such that the points winding goes ccw,
// the normal points in the direction of the viewer (same as OpenGL)
plane3_t plane(const point3_t& a, const point3_t& b, const point3_t& c) PURE_FUNC;

plane3_t planeParametric(const point3_t& a, const vec3_t& u, const vec3_t& v) PURE_FUNC;

// signed distance, positive on the side where the normal points
float distance(const plane3_t& x, const point3_t& p) PURE_FUNC;

} // namespace math

#endif
