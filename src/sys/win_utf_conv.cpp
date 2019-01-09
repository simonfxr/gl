#include "sys/win_utf_conv.hpp"

#include "err/err.hpp"

#include <Windows.h>

namespace sys {
namespace win {

std::wstring utf8To16(std::string_view str) noexcept
{
    if (str.empty())
        return {};
    auto len =
      MultiByteToWideChar(CP_UTF8, 0, str.data(), int(str.size()), nullptr, 0);
    if (!len)
        FATAL_ERR("MultiByteToWideChar failed (size)");
    std::wstring ret(size_t(len), wchar_t{});
    len = MultiByteToWideChar(
      CP_UTF8, 0, str.data(), int(str.size()), ret.data(), len);
    if (!len)
        FATAL_ERR("MultiByteToWideChar failed");
    return ret;
}

std::string utf16To8(std::wstring_view str) noexcept
{
    if (str.empty())
        return {};
    auto len = WideCharToMultiByte(
        CP_UTF8, 0, str.data(), int(str.size()), nullptr, 0, nullptr, nullptr);
    if (!len)
        FATAL_ERR("WideCharToMultiByte failed (size)");
    std::string ret(size_t(len), char16_t{});
    len = WideCharToMultiByte(
        CP_UTF8, 0, str.data(), int(str.size()), ret.data(), len, nullptr, nullptr);
    if (!len)
        FATAL_ERR("WideCharToMultiByte failed");
    ret.resize(strlen(ret.c_str()));
    return ret;
}

} // namespace win
} // namespace sys
