#ifndef BL_SHARED_PTR_HPP
#define BL_SHARED_PTR_HPP

#include "bl/core.hpp"

#include <memory>

namespace bl {

template<typename T>
using shared_ptr = std::shared_ptr<T>;

template<typename T>
using weak_ptr = std::weak_ptr<T>;

template<typename T>
using enable_shared_from_this = std::enable_shared_from_this<T>;

template<typename T, typename... Args>
inline constexpr decltype(auto)
make_shared(Args &&... args)
{
    return std::make_shared<T>(bl::forward<Args>(args)...);
}

} // namespace bl

#endif
