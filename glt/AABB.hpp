#ifndef GLT_AABB_HPP
#define GLT_AABB_HPP

#include "math/vec3.hpp"

namespace glt {

struct AABB {
    math::point3_t corner_min;
    math::point3_t corner_max;

    AABB() {
        clear();
    }
    
    void extend(const point3_t& p) {
        corner_min = math::min(p, corner_min);
        corner_max = math::max(p, corner_max);
    }

    void clear() {
        float big = 1e6;
        corner_min = math::vec3(big);
        corner_max = math::vec3(-big);
    }

    math::point3_t center() const {
        return 0.5f * (corner_min + corner_max);
    }
};

} // namespace glt

#endif

