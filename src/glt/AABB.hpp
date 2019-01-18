#ifndef GLT_AABB_HPP
#define GLT_AABB_HPP

#include "math/real.hpp"
#include "math/vec3.hpp"

namespace glt {

struct AABB
{
    math::point3_t corner_min{};
    math::point3_t corner_max{};

    constexpr AABB() { clear(); }

    constexpr void extend(const math::point3_t &p)
    {
        corner_min = math::min(p, corner_min);
        corner_max = math::max(p, corner_max);
    }

    constexpr void clear()
    {
        corner_min = math::vec3(math::REAL_MAX);
        corner_max = math::vec3(math::REAL_MIN);
    }

    constexpr math::point3_t center() const
    {
        using namespace math;
        return 0.5f * (corner_min + corner_max);
    }

    constexpr math::vec3_t dimensions() const
    {
        using namespace math;
        return corner_max - corner_min;
    }
};

} // namespace glt

#endif
