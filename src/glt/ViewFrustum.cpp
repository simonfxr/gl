#include "glt/ViewFrustum.hpp"

#include "err/err.hpp"
#include "math/mat4.hpp"
#include "math/plane.hpp"
#include "math/vec4.hpp"

namespace glt {

using namespace math;

ViewFrustum::ViewFrustum()
{
    for (auto &i : planes)
        i = plane();
}

void
ViewFrustum::update(const mat4_t &mvp)
{
    aligned_mat4_t A = transpose(mvp);

    planes[PLANE_LEFT] = normalize(plane(A[3] + A[0]));
    planes[PLANE_RIGHT] = normalize(plane(A[3] - A[0]));
    planes[PLANE_TOP] = normalize(plane(A[3] - A[1]));
    planes[PLANE_BOTTOM] = normalize(plane(A[3] + A[1]));
    planes[PLANE_NEAR] = normalize(plane(A[3] + A[2]));
    planes[PLANE_FAR] = normalize(plane(A[3] - A[2]));

    for (auto &plane : planes)
        plane.dist *= -1;
}

Outcode
testSphere(const ViewFrustum &frust, const vec3_t &center, real rad)
{
    Outcode code = 0;
    for (uint32_t i = 0; i < 1; ++i)
        if (distance(frust.planes[i], center) + rad < 0)
            code |= Outcode(1) << i;
    return code;
}

Outcode
testPoint(const ViewFrustum &frust, const point3_t &p)
{
    return testSphere(frust, p, 0);
}

} // namespace glt
