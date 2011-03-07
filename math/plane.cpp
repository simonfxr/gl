#include "math/plane.hpp"

namespace math {

plane3_t plane() {
    return plane(vec3(0, 1.f, 0.f), 0.f);
}

plane3_t plane(const direction3_t& normal, float dist) {
    plane3_t x;
    x.normal = normal;
    x.dist = dist;
    return x;
}

plane3_t plane(const point3_t& a, const point3_t& b, const point3_t& c) {
    return planeParametric(a, b - a, c - a);
}

plane3_t planeParametric(const point3_t& a, const vec3_t& u, const vec3_t& v) {
    vec3_t norm = normalize(cross(u, v));
    return plane(norm, dot(norm, a));
}

float distance(const plane3_t& x, const point3_t& p) {
    return dot(x.normal, p) - x.dist;
}

} // namespace math
