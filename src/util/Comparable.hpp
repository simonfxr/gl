#ifndef UTIL_COMPARABLE_HPP
#define UTIL_COMPARABLE_HPP

#include "defs.h"

#include <compare>
#include <type_traits>
#include <utility>

inline std::strong_ordering
chained_compare()
{
    return 0 <=> 0;
}

template<typename F, typename... Fs>
constexpr auto
chained_compare(F &&f, Fs &&...fs)
{
    auto ret = std::forward<F>(f).operator()();
    if (ret != 0)
        return ret;
    return chained_compare(std::forward<Fs>(fs)...);
}

#endif
