#ifndef PLANE_HPP
#define PLANE_HPP

#include "math/fvec.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

namespace math {

// every point x on the plane solves dot(normal, x - dist * normal) = 0
struct plane3_t
{
    union
    {
        struct
        {
            direction3 normal;
            real dist; // distance to origin
        };

        vec4 coeff;
    };

    constexpr plane3_t() : normal{}, dist{} {}
};

inline constexpr plane3_t
plane(const direction3 &normal, real dist)
{
    plane3_t P;
    P.normal = normal;
    P.dist = dist;
    return P;
}

inline constexpr plane3_t
plane()
{
    return plane(vec3(0, 1, 0), 0); // xz-plane (y = 0)
}

inline constexpr plane3_t
plane(const vec4 &coeff)
{
    return plane(vec3(coeff), coeff[3]);
}

inline plane3_t
planeParametric(const point3 &a, const vec3 &u, const vec3 &v)
{
    direction3 norm = normalize(cross(u, v));
    return plane(norm, dot(norm, a));
}

// plane through a triangle: if the triangle is viewed such that the points
// winding goes ccw, the normal points in the direction of the viewer (same as
// OpenGL)
inline plane3_t
plane(const point3 &a, const point3 &b, const point3 &c)
{
    return planeParametric(a, b - a, c - a);
}

inline plane3_t
normalize(const plane3_t &P)
{
    real l = inverseLength(P.normal);
    return plane(P.coeff * l);
}

// signed distance, positive on the side where the normal points
inline constexpr real
distance(const plane3_t &P, const point3 &a)
{
    return dot(P.normal, a) - P.dist;
}

inline point3
projectOnto(const plane3_t &P, const point3 &a)
{
    return (P.dist - dot(P.normal, a)) * P.normal + a;
}

inline direction3
transformNormal(const mat3_t &A, const direction3 &n)
{
    // transforming the normal like a normal vector might break
    // orthogonality for non orthonormal transforms
    // we have 2 options:
    //  i)  n' = inverse(transpose(A)) * n
    //  ii) transform the basis vectors to build a new basis
    // option 2 might be faster for a single plane
    // to transform many planes option 1 might be faster

    mat3_t basis = coordinateSystem(n);
    return normalize(cross(transform(A, basis[1]), transform(A, basis[2])));
}

inline plane3_t
transform(const mat3_t &A, const plane3_t &P)
{
    direction3 n = transformNormal(A, P.normal);
    point3 a = transform(A, P.normal * P.dist);
    return plane(n, dot(a, n));
}

inline plane3_t
transform(const mat4_t &A, const plane3_t &P)
{
    direction3 n = transformNormal(mat3(A), P.normal);
    point3 a = transformPoint(A, P.normal * P.dist);
    return plane(n, dot(a, n));
}

} // namespace math

#endif
