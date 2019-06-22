#ifndef BL_SWAP_HPP
#define BL_SWAP_HPP

#include "bl/core.hpp"
#include "bl/type_traits.hpp"

namespace bl {

template<typename T>
void
swap(T &x, T &y)
{
    static_assert(is_nothrow_move_assignable_v<T> &&
                  is_nothrow_move_constructible_v<T>);
    T tmp(bl::move(x));
    x = bl::move(y);
    y = bl::move(tmp);
}

} // namespace bl

#endif
