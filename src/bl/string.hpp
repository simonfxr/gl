#ifndef BL_STRING_HPP
#define BL_STRING_HPP

#include <bl/conf.hpp>

#include <string>

namespace bl {

template<class CharT, class Traits, class Allocator>
using basic_string = std::basic_string<CharT, Traits, Allocator>;

using string = basic_string<char, std::char_traits<char>, std::allocator<char>>;

} // namespace bl

#endif // BL_STRING_HPP
