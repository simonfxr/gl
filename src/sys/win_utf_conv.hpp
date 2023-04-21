#ifndef SYS_WIN_UTF_CONV_HPP
#define SYS_WIN_UTF_CONV_HPP

#include "sys/conf.hpp"

#include <bl/string.hpp>
#include <bl/string_view.hpp>

namespace sys {
namespace win {

std::wstring utf8To16(bl::string_view) noexcept;

bl::string utf16To8(std::wstring_view) noexcept;

} // namespace win
} // namespace sys

#endif
