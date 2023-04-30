#ifndef BL_STRING_LITERALS_HPP
#define BL_STRING_LITERALS_HPP

namespace bl::literals {

constexpr string operator""_s(const string::value_type *s, usize n) noexcept
{
    return string{ s, s + n };
}

} // namespace bl::literals

#endif // BL_STRING_LITERALS_HPP
