#include "glt/ViewFrustum.hpp"

namespace glt {

using namespace math;

ViewFrustum::ViewFrustum() {
    for (uint32 i = 0; i < 6; ++i)
        planes[i] = plane();
}

Outcode testSphere(const ViewFrustum& frust, const vec3_t& center, float rad) {
    Outcode code = 0;

    for (uint32 i = 0; i < VIEW_FRUSTUM_PLANES; ++i)
        if (distance(frust.planes[i], center) + rad < 0)
            code |= 1ul << i;

    return code;    
}

Outcode testPoint(const ViewFrustum& frust, const point3_t& p) {
    return testSphere(frust, p, 0.f);
}

} // namespace glt

