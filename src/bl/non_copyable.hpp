#ifndef BL_NON_COPYABLE_HPP
#define BL_NON_COPYABLE_HPP

#include "defs.h"

namespace bl {

struct non_copyable
{
    constexpr non_copyable() = default;
    constexpr non_copyable(non_copyable &&) noexcept = default;
    non_copyable(const non_copyable &) = delete;
    non_copyable &operator=(const non_copyable &) = delete;
};

} // namespace bl

#endif
