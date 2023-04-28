#ifndef BL_STRING_VIEW_HPP
#define BL_STRING_VIEW_HPP

#include <bl/span.hpp>

#include <string_view>

#define BL_STRING_VIEW(...) "" __VA_ARGS__ ""_sv

namespace bl {

template<typename CharT>
using basic_string_view = bl::span<const CharT>;

using string_view = basic_string_view<string_byte>;

template<size_t N>
    requires(N > 0)
BL_consteval string_view string_view_literal(const char (&literal)[N]) noexcept
{
    return string_view{ literal, N - 1 };
}

using bytes_view = basic_string_view<byte>;

} // namespace bl

#include <bl/_string_view_literals.hpp>

#endif // BL_STRING_VIEW_HPP
