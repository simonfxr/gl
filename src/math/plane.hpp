#ifndef PLANE_HPP
#define PLANE_HPP

#include "math/mdefs.hpp"
#include "math/vec3/type.hpp"
#include "math/vec4/type.hpp"
#include "math/mat3/type.hpp"
#include "math/mat4/type.hpp"

namespace math {

// every point x on the plane solves dot(normal, x - dist * normal) = 0
struct plane3_t {

    union {
        struct {
            direction3_t normal;
            float dist; // distance to origin
        };

        vec4_t coeff;
    };
};

} // namespace math

MATH_BEGIN_NAMESPACE

plane3_t plane() PURE_FUNC;

plane3_t plane(const vec4_t& coeff) PURE_FUNC;

plane3_t plane(const direction3_t& normal, float dist) PURE_FUNC;

// plane through a triangle: if the triangle is viewed such that the points winding goes ccw,
// the normal points in the direction of the viewer (same as OpenGL)
plane3_t plane(const point3_t& a, const point3_t& b, const point3_t& c) PURE_FUNC;

plane3_t planeParametric(const point3_t& a, const vec3_t& u, const vec3_t& v) PURE_FUNC;

plane3_t normalize(const plane3_t& P) PURE_FUNC;

// signed distance, positive on the side where the normal points
float distance(const plane3_t& P, const point3_t& a) PURE_FUNC;

point3_t projectOnto(const plane3_t& P, const point3_t& a) PURE_FUNC;

direction3_t transformNormal(const mat3_t& A, const direction3_t& n) PURE_FUNC;

plane3_t transform(const mat3_t& A, const plane3_t& P) PURE_FUNC;

plane3_t transform(const mat4_t& A, const plane3_t& P) PURE_FUNC;

MATH_END_NAMESPACE

#if defined(MATH_INLINE)
#define MATH_PLANE_INLINE
#include "math/plane.cpp"
#endif

#endif
