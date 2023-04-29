#ifndef BL_TUPLE_HPP
#define BL_TUPLE_HPP

#include <bl/conf.hpp>

#include <tuple>

namespace bl {
template<typename... Ts>
using tuple = std::tuple<Ts...>;

template<typename... Ts>
BL_inline constexpr tuple<Ts &...>
tie(Ts &...xs) noexcept
{
    return tuple<Ts &...>(xs...);
}

} // namespace bl

#endif // BL_TUPLE_HPP
