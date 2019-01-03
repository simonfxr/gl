#ifndef PLANE_HPP
#define PLANE_HPP

#include "math/mat3/type.hpp"
#include "math/mat4/type.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace math {

// every point x on the plane solves dot(normal, x - dist * normal) = 0
struct plane3_t
{
    union
    {
        struct
        {
            direction3_t normal;
            real dist; // distance to origin
        };

        vec4_t coeff;
    };

    constexpr plane3_t() : normal{}, dist{} {}
};

} // namespace math

namespace math {

constexpr inline plane3_t
plane();

constexpr inline plane3_t
plane(const vec4_t &coeff);

constexpr inline plane3_t
plane(const direction3_t &normal, real dist);

// plane through a triangle: if the triangle is viewed such that the points
// winding goes ccw, the normal points in the direction of the viewer (same as
// OpenGL)
inline plane3_t
plane(const point3_t &a, const point3_t &b, const point3_t &c);

inline plane3_t
planeParametric(const point3_t &a, const vec3_t &u, const vec3_t &v);

inline plane3_t
normalize(const plane3_t &P);

// signed distance, positive on the side where the normal points
constexpr inline real
distance(const plane3_t &P, const point3_t &a);

constexpr inline point3_t
projectOnto(const plane3_t &P, const point3_t &a);

inline direction3_t
transformNormal(const mat3_t &A, const direction3_t &n);

inline plane3_t
transform(const mat3_t &A, const plane3_t &P);

inline plane3_t
transform(const mat4_t &A, const plane3_t &P);

} // namespace math

#if defined(MATH_INLINE)
#define MATH_PLANE_INLINE
#include "math/plane.cpp"
#endif

#endif
