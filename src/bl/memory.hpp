#ifndef BL_MEMORY_HPP
#define BL_MEMORY_HPP

#include "bl/core.hpp"
#include "bl/iterator.hpp"
#include "bl/new.hpp"
#include "bl/type_traits.hpp"

namespace bl {

template<class InputIt, class ForwardIt>
inline ForwardIt
uninitialized_copy(InputIt first, InputIt last, ForwardIt d_first)
{
    using T = std::remove_cv_t<std::remove_reference_t<decltype(*d_first)>>;
    for (; first != last; ++first, ++d_first)
        new (addressof(*d_first)) T(*first);
    return d_first;
}

template<class ForwardIt, class T>
inline void
uninitialized_fill(ForwardIt first, ForwardIt last, const T &value)
{
    using U = std::remove_cv_t<std::remove_reference_t<decltype(*first)>>;
    for (; first != last; ++first)
        new (addressof(*first)) U(value);
}

template<class InputIt, class ForwardIt>
inline ForwardIt
uninitialized_move(InputIt first, InputIt last, ForwardIt d_first)
{
    using T = std::remove_cv_t<std::remove_reference_t<decltype(*d_first)>>;
    for (; first != last; ++first, ++d_first)
        new (addressof(*d_first)) T(std::move(*first));
    return d_first;
}

template<class InputIt, class ForwardIt>
inline ForwardIt
uninitialized_destructive_move(InputIt first, InputIt last, ForwardIt d_first)
{
    using T = std::remove_cv_t<std::remove_reference_t<decltype(*d_first)>>;
    for (; first != last; ++first, ++d_first) {
        new (addressof(*d_first)) T(std::move(*first));
        destroy_at(addressof(*first));
    }
    return d_first;
}

template<class ForwardIt>
inline void
uninitialized_default_construct(ForwardIt first, ForwardIt last)
{
    using T = typename iterator_traits<ForwardIt>::value_type;
    if constexpr (!std::is_trivially_default_constructible_v<T>) {
        for (; first != last; ++first)
            new (addressof(*first)) T;
    }
}

} // namespace bl

#endif
