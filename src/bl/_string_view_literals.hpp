#ifndef BL_STRING_VIEW_LITERALS_HPP
#define BL_STRING_VIEW_LITERALS_HPP

namespace bl::literals {

constexpr string_view operator""_sv(const string_view::value_type *s,
                                    std::size_t n) noexcept
{
    return string_view{ s, n };
}

constexpr string_view operator""_k(const string_view::value_type *s,
                                   std::size_t n) noexcept
{
    return string_view{ s, n };
}

// constexpr bytes_view operator""_bv(co t har *s, std::size_t n) noexcept
// {
//     return bytes_view{ s, n };
// }

} // namespace bl::literals

#endif // BL_STRING_VIEW_LITERALS_HPP
