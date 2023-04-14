#ifndef GLT_AABB_HPP
#define GLT_AABB_HPP

#include "math/fvec.hpp"
#include "math/real.hpp"

namespace glt {

struct AABB
{
    math::point3 corner_min{};
    math::point3 corner_max{};

    constexpr AABB() { clear(); }

    constexpr void extend(const math::point3 &p)
    {
        corner_min = math::min(p, corner_min);
        corner_max = math::max(p, corner_max);
    }

    constexpr void clear()
    {
        corner_min = math::vec3(math::REAL_MAX);
        corner_max = math::vec3(math::REAL_MIN);
    }

    constexpr math::point3 center() const
    {
        using namespace math;
        return 0.5f * (corner_min + corner_max);
    }

    constexpr math::vec3 dimensions() const
    {
        using namespace math;
        return corner_max - corner_min;
    }
};

} // namespace glt

#endif
