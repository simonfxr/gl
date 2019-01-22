#ifndef BL_UNIQUE_PTR_HPP
#define BL_UNIQUE_PTR_HPP

#include "bl/core.hpp"

#include <memory>

namespace bl {

template<typename... Args>
using unique_ptr = std::unique_ptr<Args...>;

template<typename T, typename... Args>
inline constexpr decltype(auto)
make_unique(Args &&... args)
{
    return std::make_unique<T>(bl::forward<Args>(args)...);
}

} // namespace bl

#endif
