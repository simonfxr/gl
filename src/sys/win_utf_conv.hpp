#ifndef SYS_WIN_UTF_CONV_HPP
#define SYS_WIN_UTF_CONV_HPP

#include "sys/conf.hpp"

#include <bl/string.hpp>
#include <bl/string_view.hpp>

namespace sys {
namespace win {

using platform_string = bl::basic_string<wchar_t>;
using platform_string_view = bl::basic_string_view<wchar_t>;

platform_string utf8To16(bl::string_view) noexcept;

bl::string utf16To8(platform_string_view) noexcept;

} // namespace win
} // namespace sys

#endif
