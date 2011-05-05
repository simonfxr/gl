#ifndef GLT_AABB_HPP
#define GLT_AABB_HPP

#include "math/math.hpp"
#include "math/vec3.hpp"

namespace glt {

struct AABB {

    math::point3_t corner_min;
    math::point3_t corner_max;

    AABB() {
        clear();
    }
    
    void extend(const math::point3_t& p) {
        corner_min = math::min(p, corner_min);
        corner_max = math::max(p, corner_max);
    }

    void clear() {
        corner_min = math::vec3(math::POS_INF);
        corner_max = math::vec3(math::NEG_INF);
    }

    math::point3_t center() const {
        using namespace math;
        return 0.5f * (corner_min + corner_max);
    }

    math::vec3_t dimensions() const {
        using namespace math;
        return corner_max - corner_min;
    }
};

} // namespace glt

#endif
