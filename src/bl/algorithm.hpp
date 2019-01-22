#ifndef BL_ALGORITHM_HPP
#define BL_ALGORITHM_HPP

#include "bl/core.hpp"

namespace bl {

template<typename InputIt, typename OutputIt>
constexpr OutputIt
copy(InputIt first, InputIt last, OutputIt d_first)
{
    for (; first != last; ++first, ++d_first)
        *d_first = *first;
    return d_first;
}

template<typename InputIt, typename OutputIt>
constexpr OutputIt
move(InputIt first, InputIt last, OutputIt d_first)
{
    for (; first != last; ++first, ++d_first)
        *d_first = std::move(*first);
    return d_first;
}

template<typename InputIt, typename T>
inline constexpr InputIt
find(InputIt first, InputIt last, const T &value)
{
    for (; first != last; ++first)
        if (value == *first)
            break;
    return first;
}

template<typename U, typename T>
inline constexpr const U *
rfind(const U *first, const U *last, const T &value)
{
    while (first != last)
        if (value == *--last)
            return last;
    return nullptr;
}

template<typename InputIt, typename T>
inline constexpr InputIt
find_sentinel(InputIt first, const T &value)
{
    while (*first != value)
        ++first;
    return first;
}

} // namespace bl

#endif
