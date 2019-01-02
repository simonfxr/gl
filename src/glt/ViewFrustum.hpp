#ifndef GLT_VIEWING_FRUSTUM_HPP
#define GLT_VIEWING_FRUSTUM_HPP

#include "glt/conf.hpp"
#include "math/mat4/type.hpp"
#include "math/plane.hpp"
#include "math/vec3/type.hpp"

namespace glt {

using Outcode = uint32_t;

enum PlaneIndex
{
    PLANE_LEFT = 0,
    PLANE_RIGHT = 1,
    PLANE_TOP = 2,
    PLANE_BOTTOM = 3,
    PLANE_NEAR = 4,
    PLANE_FAR = 5
};

namespace {

inline constexpr uint32_t VIEW_FRUSTUM_PLANES = 6;

#define BIT(k) (1ul << uint32_t(k))

inline constexpr Outcode CLIP_LEFT = BIT(PLANE_LEFT);
inline constexpr Outcode CLIP_RIGHT = BIT(PLANE_RIGHT);
inline constexpr Outcode CLIP_TOP = BIT(PLANE_TOP);
inline constexpr Outcode CLIP_BOTTOM = BIT(PLANE_BOTTOM);
inline constexpr Outcode CLIP_NEAR = BIT(PLANE_NEAR);
inline constexpr Outcode CLIP_FAR = BIT(PLANE_FAR);

#undef BIT

inline constexpr Outcode CLIP_ALL_MASK = (1ul << 6) - 1;

} // namespace

// a four sided pyramid frustum, used to represent a viewing volume
struct GLT_API ViewFrustum
{
    // clip planes in world coordinates, normales point in the volume
    math::plane3_t planes[VIEW_FRUSTUM_PLANES]{};

    ViewFrustum();

    void update(const math::mat4_t &mvpMatrix);
};

GLT_API Outcode
testSphere(const ViewFrustum &frust,
           const math::point3_t &center,
           math::real rad);

GLT_API Outcode
testPoint(const ViewFrustum &frust, const math::point3_t &p);

inline bool
clipped(Outcode code)
{
    return (code & CLIP_ALL_MASK) != 0;
}

} // namespace glt

#endif
