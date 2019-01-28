#ifndef BL_MEMORY_HPP
#define BL_MEMORY_HPP

#include "bl/core.hpp"
#include "bl/iterator.hpp"
#include "bl/new.hpp"
#include "bl/type_traits.hpp"

namespace bl {

template<typename InputIt, typename ForwardIt>
inline ForwardIt
uninitialized_copy(InputIt first, InputIt last, ForwardIt d_first)
{
    using T = std::remove_cv_t<std::remove_reference_t<decltype(*d_first)>>;
    for (; first != last; ++first, ++d_first)
        new (addressof(*d_first)) T(*first);
    return d_first;
}

template<typename InputIt, typename ForwardIt>
inline ForwardIt
uninitialized_move(InputIt first, InputIt last, ForwardIt d_first)
{
    using T = std::remove_cv_t<std::remove_reference_t<decltype(*d_first)>>;
    for (; first != last; ++first, ++d_first)
        new (addressof(*d_first)) T(std::move(*first));
    return d_first;
}

template<typename AssignTag, typename InputIt, typename ForwardIt>
inline ForwardIt
uninitialized_assign(AssignTag assign,
                     InputIt first,
                     InputIt last,
                     ForwardIt d_first)
{
    using T = std::remove_cv_t<std::remove_reference_t<decltype(*d_first)>>;
    for (; first != last; ++first, ++d_first)
        initialize(assign, addressof(*d_first), T(*first));
    return d_first;
}

template<typename ForwardIt>
inline void
uninitialized_default_construct(ForwardIt first, ForwardIt last)
{
    using T = typename iterator_traits<ForwardIt>::value_type;
    for (; first != last; ++first)
        new (addressof(*first)) T;
}

template<typename ForwardIt, typename T>
inline void
uninitialized_fill(ForwardIt first, ForwardIt last, const T &value)
{
    using U = std::remove_cv_t<std::remove_reference_t<decltype(*first)>>;
    for (; first != last; ++first)
        new (addressof(*first)) U(value);
}

template<typename InputIt, typename ForwardIt>
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

} // namespace bl

#endif
