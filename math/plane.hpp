#ifndef PLANE_HPP
#define PLANE_HPP

#include "math/defs.hpp"
#include "math/vec3.hpp"

namespace math {

struct plane3_t {
    direction3_t normal;
    float dist;
};

plane3_t plane(const direction3_t& normal, float dist) PURE_FUNC;

plane3_t plane(const point3_t& a, const point3_t& b, const point3_t& c) PURE_FUNC;

plane3_t planeParametric(const point3_t& a, const vec3_t& u, const vec3_t& v) PURE_FUNC;

float distance(const plane3_t& x, const point3_t& p) PURE_FUNC;

} // namespace math

#endif
