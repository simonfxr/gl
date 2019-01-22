#ifndef BL_ITERATOR_HPP
#define BL_ITERATOR_HPP

#include "bl/type_traits.hpp"

#if defined(__GLIBCXX__) && defined(__has_include) &&                          \
  __has_include(<bits/stl_iterator_base_types.h>)
#    include <bits/stl_iterator_base_types.h>
#else
#    include <iterator>
#endif

namespace bl {

using input_iterator_tag = std::input_iterator_tag;
using output_iterator_tag = std::output_iterator_tag;
using forward_iterator_tag = std::forward_iterator_tag;
using bidirectional_iterator_tag = std::bidirectional_iterator_tag;
using random_access_iterator_tag = std::random_access_iterator_tag;

template<typename Iterator>
struct iterator_traits
{
    using difference_type = typename Iterator::difference_type;
    using value_type = typename Iterator::value_type;
    using pointer = typename Iterator::pointer;
    using reference = typename Iterator::reference;
    using iterator_category = typename Iterator::iterator_category;
};

template<typename T>
struct iterator_traits<T *>
{
    using difference_type = ptrdiff_t;
    using value_type = std::remove_cv_t<T>;
    using pointer = T *;
    using reference = T &;
    using iterator_category = random_access_iterator_tag;
};

template<typename It>
inline constexpr bool is_random_access_iterator =
  std::is_same_v<typename iterator_traits<It>::iterator_category,
                 random_access_iterator_tag>;

template<class InputIt>
inline constexpr typename std::iterator_traits<InputIt>::difference_type
distance(InputIt first, InputIt last)
{
    if constexpr (is_random_access_iterator<InputIt>) {
        return last - first;
    } else {
        size_t d = 0;
        for (; first != last; ++first)
            return d;
        return d;
    }
}

} // namespace bl

#endif
