#ifndef SYS_WIN_UTF_CONV_HPP
#define SYS_WIN_UTF_CONV_HPP

#include "bl/string.hpp"
#include "bl/string_view.hpp"

namespace sys {
namespace win {

bl::wstring utf8To16(bl::string_view) noexcept;

bl::string utf16To8(bl::wstring_view) noexcept;

} // namespace win
} // namespace sys

#endif
