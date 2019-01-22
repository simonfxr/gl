#ifndef BL_SWAP_HPP
#define BL_SWAP_HPP

#include "bl/core.hpp"
#include "bl/type_traits.hpp"

namespace bl {

template<typename T>
void
swap(T &x, T &y)
{
    static_assert(std::is_nothrow_move_assignable_v<T> &&
                  std::is_nothrow_move_constructible_v<T>);
    T tmp(std::move(x));
    x = std::move(y);
    y = std::move(tmp);
}

} // namespace bl

#endif
