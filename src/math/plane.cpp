#include "math/mdefs.hpp"

#if defined(MATH_PLANE_INLINE) || !defined(MATH_INLINE)

#include "math/mat3.hpp"
#include "math/mat4.hpp"
#include "math/plane.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

MATH_BEGIN_NAMESPACE

plane3_t
plane()
{
    return plane(vec3(0, 1, 0), 0); // xz-plane (y = 0)
}

plane3_t
plane(const vec4_t &coeff)
{
    plane3_t P;
    P.coeff = coeff;
    return P;
}

plane3_t
plane(const direction3_t &normal, real dist)
{
    plane3_t P;
    P.normal = normal;
    P.dist = dist;
    return P;
}

plane3_t
plane(const point3_t &a, const point3_t &b, const point3_t &c)
{
    return planeParametric(a, b - a, c - a);
}

plane3_t
planeParametric(const point3_t &a, const vec3_t &u, const vec3_t &v)
{
    direction3_t norm = normalize(cross(u, v));
    return plane(norm, dot(norm, a));
}

plane3_t
normalize(const plane3_t &P)
{
    real l = inverseLength(P.normal);
    return plane(P.coeff * l);
}

real
distance(const plane3_t &P, const point3_t &a)
{
    return dot(P.normal, a) - P.dist;
}

point3_t
projectOnto(const plane3_t &P, const point3_t &a)
{
    return (P.dist - dot(P.normal, a)) * P.normal + a;
}

direction3_t
transformNormal(const mat3_t &A, const direction3_t &n)
{
    // transforming the normal like a normal vector might break
    // orthogonality for non orthonormal transforms
    // we have 2 options:
    //  i)  n' = inverse(transpose(A)) * n
    //  ii) transform the basis vectors to build a new basis
    // option 2 might be faster for a single plane
    // to transform many planes option 1 might be faster

    mat3_t basis = coordinateSystem(n);
    return normalize(
      cross(transformVector(A, basis[1]), transformVector(A, basis[2])));
}

plane3_t
transform(const mat3_t &A, const plane3_t &P)
{
    direction3_t n = transformNormal(A, P.normal);
    point3_t a = transformPoint(A, P.normal * P.dist);
    return plane(n, dot(a, n));
}

plane3_t
transform(const mat4_t &A, const plane3_t &P)
{
    direction3_t n = transformNormal(mat3(A), P.normal);
    point3_t a = transformPoint(A, P.normal * P.dist);
    return plane(n, dot(a, n));
}

MATH_END_NAMESPACE

#endif
