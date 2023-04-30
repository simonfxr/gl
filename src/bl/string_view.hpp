#ifndef BL_STRING_VIEW_HPP
#define BL_STRING_VIEW_HPP

#include <bl/core.hpp>
#include <bl/span.hpp>

#include <cstring>
#include <string_view>

#define BL_STRING_VIEW(...) "" __VA_ARGS__ ""_sv

namespace bl {

template<typename CharT>
using basic_string_view = bl::span<const CharT>;

using string_view = basic_string_view<cbyte>;

template<usize N>
    requires(N > 0)
BL_consteval string_view string_view_literal(const char (&literal)[N]) noexcept
{
    return string_view{ literal, N - 1 };
}

constexpr string_view
from_cstring(const char *s) noexcept
{
    if (!s)
        return string_view{};
    if (std::is_constant_evaluated()) {
        usize l = 0;
        for (auto p = s; *p; ++p)
            ++l;
        return string_view{ s, l };
    } else {
        return string_view{ s, std::strlen(s) };
    }
}

using bytes_view = basic_string_view<byte>;

} // namespace bl

#include <bl/_string_view_literals.hpp>

#endif // BL_STRING_VIEW_HPP
