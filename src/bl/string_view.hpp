#ifndef BL_STRING_VIEW_HPP
#define BL_STRING_VIEW_HPP

#include <bl/conf.hpp>

#include <string_view>

namespace bl {

template<typename CharT, typename Traits = std::char_traits<CharT>>
using basic_string_view = std::basic_string_view<CharT, Traits>;

using string_view = basic_string_view<char>;

} // namespace bl

#endif // BL_STRING_VIEW_HPP
