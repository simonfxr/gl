#ifndef BL_STRING_HPP
#define BL_STRING_HPP

#include <bl/core.hpp>
#include <bl/string_view.hpp>

#include <string>

namespace bl {

inline constexpr struct explicit_arg_t
{
} explicit_arg;

template<class CharT, typename Allocator = std::allocator<CharT>>
struct basic_string
  : private std::basic_string<CharT, std::char_traits<CharT>, Allocator>
{
    using Base = std::basic_string<CharT, std::char_traits<CharT>, Allocator>;

    using typename Base::value_type;

    using Base::append;
    using std::basic_string<CharT, std::char_traits<CharT>, Allocator>::
      basic_string;
    using Base::begin;
    using Base::c_str;
    using Base::data;
    using Base::empty;
    using Base::end;
    using Base::find;
    using Base::npos;
    using Base::rbegin;
    using Base::rend;
    using Base::reserve;
    using Base::resize;
    using Base::rfind;
    using Base::size;
    using Base::operator[];

    template<typename Elem, usize N>
    basic_string(Elem (&)[N]) = delete;

    template<usize N>
    basic_string(CharT (&)[N]) = delete;

    explicit constexpr basic_string(explicit_arg_t, const CharT *p) : Base(p) {}
    explicit constexpr basic_string(explicit_arg_t, CharT *p) : Base(p) {}

    basic_string(const CharT *p) = delete;
    basic_string(CharT *p) = delete;

    constexpr basic_string(const basic_string_view<CharT> &rhs)
      : basic_string(rhs.begin(), rhs.end())
    {}

    /* implicit */ constexpr operator basic_string_view<CharT>() noexcept
    {
        return { data(), size() };
    }

    friend constexpr bool operator==(
      const basic_string<CharT, Allocator> &lhs,
      const basic_string<CharT, Allocator> &rhs) noexcept
    {
        return static_cast<basic_string_view<CharT>>(lhs) ==
               static_cast<basic_string_view<CharT>>(rhs);
    }

    friend constexpr bool operator==(
      const basic_string<CharT, Allocator> &lhs,
      const basic_string_view<CharT> &rhs) noexcept
    {
        return static_cast<basic_string_view<CharT>>(lhs) == rhs;
    }

    friend constexpr bool operator==(
      const basic_string_view<CharT> &lhs,
      const basic_string<CharT, Allocator> &rhs) noexcept
    {
        return lhs == static_cast<basic_string_view<CharT>>(rhs);
    }

    friend constexpr auto operator<=>(const basic_string &lhs,
                                      const basic_string &rhs) noexcept
    {
        return static_cast<basic_string_view<CharT>>(lhs) <=>
               static_cast<basic_string_view<CharT>>(rhs);
    }
};

using string = basic_string<cbyte>;

using bytes = basic_string<byte>;

} // namespace bl

#include <bl/_string_literals.hpp>

template<typename CharT, typename Allocator>
struct std::hash<bl::basic_string<CharT, Allocator>>
{
    constexpr bl::usize operator()(
      const bl::basic_string<CharT, Allocator> &s) const noexcept
    {
        using SV = std::basic_string_view<CharT, std::char_traits<CharT>>;
        return std::hash<SV>{}(SV{ s.data(), s.size() });
    }
};

#endif // BL_STRING_HPP
