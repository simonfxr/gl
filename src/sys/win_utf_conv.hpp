#ifndef SYS_WIN_UTF_CONV_HPP
#define SYS_WIN_UTF_CONV_HPP

#include "sys/conf.hpp"

#include <string>
#include <string_view>

namespace sys {
namespace win {

std::wstring utf8To16(std::string_view) noexcept;

std::string utf16To8(std::wstring_view) noexcept;

}
} // namespace sys

#endif
