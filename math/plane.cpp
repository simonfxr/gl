#if defined(MATH_PLANE_INLINE) || !defined(MATH_INLINE)

#include "math/plane.hpp"
#include "math/vec3.hpp"
#include "math/mat3.hpp"
#include "math/mat4.hpp"

MATH_BEGIN_NAMESPACE

plane3_t plane() {
    return plane(vec3(0, 1.f, 0.f), 0.f); // xz-plane (y = 0)
}

plane3_t plane(const direction3_t& normal, float dist) {
    plane3_t P;
    P.normal = normal;
    P.dist = dist;
    return P;
}

plane3_t plane(const point3_t& a, const point3_t& b, const point3_t& c) {
    return planeParametric(a, b - a, c - a);
}

plane3_t planeParametric(const point3_t& a, const vec3_t& u, const vec3_t& v) {
    direction3_t norm = normalize(cross(u, v));
    return plane(norm, dot(norm, a));
}

float distance(const plane3_t& P, const point3_t& a) {
    return dot(P.normal, a) - P.dist;
}

point3_t projectOnto(const plane3_t& P, const point3_t& a) {
//    return a + (-dot(P.normal, a) + P.dist) * P.normal;
    point3_t b = P.normal * P.dist;
    point3_t a_par = dot(P.normal, a) * P.normal;
    return b + a - a_par;
}

plane3_t transform(const mat3_t& A, const plane3_t& P) {
    point3_t a = transformPoint(A, P.normal * P.dist);
    direction3_t n = normalize(transformVector(A, P.normal));
    return plane(n, dot(a, n));
}

plane3_t transform(const mat4_t& A, const plane3_t& P) {
    point3_t a = transformPoint(A, P.normal * P.dist);
    direction3_t n = normalize(transformVector(A, P.normal));
    return plane(n, dot(a, n));
}

MATH_END_NAMESPACE

#endif
