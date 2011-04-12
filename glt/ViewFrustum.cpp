#include "glt/ViewFrustum.hpp"
#include "glt/utils.hpp"

namespace glt {

using namespace math;

ViewFrustum::ViewFrustum() {
    for (uint32 i = 0; i < 6; ++i)
        planes[i] = plane();
}

void ViewFrustum::updateView(const mat4_t& viewToWorld, const mat4_t& projection) {
    UNUSED(viewToWorld); UNUSED(projection);
    // FIXME: implement!
}

Outcode testSphere(const ViewFrustum& frust, const vec3_t& center, float rad) {

    UNUSED(frust); UNUSED(center); UNUSED(rad);

    // FIXME: implement!
    ERROR_ONCE("not yet implemented");

    return 0;
    
    // Outcode code = 0;

    // for (uint32 i = 0; i < VIEW_FRUSTUM_PLANES; ++i)
    //     if (distance(frust.planes[i], center) + rad < 0)
    //         code |= 1ul << i;

    // return code;    
}

Outcode testPoint(const ViewFrustum& frust, const point3_t& p) {
    return testSphere(frust, p, 0.f);
}

} // namespace glt

