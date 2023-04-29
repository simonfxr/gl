#ifndef BL_SPAN_HPP
#define BL_SPAN_HPP

#include <bl/conf.hpp>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>

namespace bl {

inline constexpr size_t dynamic_extent = std::numeric_limits<size_t>::max();

using string_byte = char;
using byte = uint8_t;

template<typename T, size_t Extent = dynamic_extent>
struct span : private std::span<T, Extent>
{
    using Base = std::span<T, Extent>;

    using typename Base::element_type;
    using typename Base::size_type;
    using typename Base::value_type;

    static inline constexpr size_type npos = static_cast<size_type>(-1);
    static inline constexpr size_type extent = Base::extent;

    using Base::begin;
    using Base::data;
    using Base::empty;
    using Base::end;
    using Base::rbegin;
    using Base::rend;
    using Base::size;
    using Base::size_bytes;
    using Base::span;
    using Base::operator[];

    template<typename Elem, size_t N>
    span(Elem (&)[N]) = delete;

    template<size_t N>
    span(T (&)[N]) = delete;

    template<size_t Count>
    constexpr span<element_type, Count> first() const
    {
        return from_std(Base::template first<Count>());
    }

    template<size_t Count>
    constexpr span<element_type, Count> last() const
    {
        return from_std(Base::template last<Count>());
    }

    template<size_t Offset, size_t Count = dynamic_extent>
    constexpr auto subspan() const
    {
        return from_std(Base::template subspan<Offset, Count>());
    }

    constexpr span<element_type, dynamic_extent> first(size_type count) const
    {
        return from_std(Base::first(count));
    }
    constexpr span<element_type, dynamic_extent> last(size_type count) const
    {
        return from_std(Base::last(count));
    }

    constexpr span<element_type, dynamic_extent> subspan(
      size_type offset,
      size_type count = dynamic_extent) const
    {
        return from_std(Base::subspan(offset, count));
    }

    constexpr size_type find(T ch, size_type pos = 0) const noexcept
    {
        using CharT = std::decay_t<T>;
        using SV = std::basic_string_view<CharT, std::char_traits<CharT>>;
        return SV{ data(), size() }.find(ch, pos);
    }

    constexpr size_type rfind(T ch, size_type pos = npos) const noexcept
    {
        using CharT = std::decay_t<T>;
        using SV = std::basic_string_view<CharT, std::char_traits<CharT>>;
        return SV{ data(), size() }.rfind(ch, pos);
    }

    friend constexpr bool operator==(const span &lhs, const span &rhs) noexcept
    {
        using CharT = std::decay_t<T>;
        if constexpr (std::is_integral_v<CharT>) {
            using SV = std::basic_string_view<CharT, std::char_traits<CharT>>;
            return SV{ lhs.data(), lhs.size() } == SV{ rhs.data(), rhs.size() };
        } else {
            return lhs.size() == rhs.size() &&
                   std::equal(lhs.begin(), lhs.end(), rhs.begin());
        }
    }

    friend constexpr auto operator<=>(const span &lhs, const span &rhs) noexcept
    {
        using CharT = std::decay_t<T>;
        if constexpr (std::is_integral_v<CharT>) {
            using SV = std::basic_string_view<CharT, std::char_traits<CharT>>;
            return SV{ lhs.data(), lhs.size() }.compare(
                     SV{ rhs.data(), rhs.size() }) <=> 0;
        } else {
            auto f1 = lhs.begin();
            auto l1 = lhs.end();
            auto f2 = rhs.begin();
            auto l2 = rhs.end();

            using ret_t = decltype(*f1 <=> *f2);

            bool exhaust1 = (f1 == l1);
            bool exhaust2 = (f2 == l2);
            for (; !exhaust1 && !exhaust2;
                 exhaust1 = (++f1 == l1), exhaust2 = (++f2 == l2))
                if (auto c = *f1 <=> *f2; c != 0)
                    return c;

            ret_t ret = !exhaust1   ? std::strong_ordering::greater
                        : !exhaust2 ? std::strong_ordering::less
                                    : std::strong_ordering::equal;
            return ret;
        }
    }

private:
    template<typename U, size_t Extent2>
    static constexpr span<U, Extent2> from_std(std::span<U, Extent2> s) noexcept
    {
        return { s.data(), s.size() };
    }
}; // namespace bl

template<typename It, typename EndOrSize>
span(It, EndOrSize) -> span<std::remove_reference_t<std::iter_reference_t<It>>>;

template<typename T, size_t N>
span(T (&)[N]) -> span<T, N>;

template<typename T, size_t N>
span(std::array<T, N> &) -> span<T, N>;

template<typename T, size_t N>
span(const std::array<T, N> &) -> span<const T, N>;

template<typename T>
using iterator_t = decltype(std::begin(bl_declval(T &)));

template<typename R>
using range_reference_t = std::iter_reference_t<iterator_t<R>>;

template<typename R>
span(R &&) -> span<std::remove_reference_t<range_reference_t<R>>>;

template<typename T, size_t Extent>
span<const byte, Extent == dynamic_extent ? dynamic_extent : Extent * sizeof(T)>
as_bytes(const span<T, Extent> &s) noexcept
{
    return { reinterpret_cast<const byte *>(s.data()), s.size_bytes() };
}

template<typename T, size_t Extent>
span<byte, Extent == dynamic_extent ? dynamic_extent : Extent * sizeof(T)>
as_writable_bytes(const span<T, Extent> &s) noexcept
{
    return { reinterpret_cast<byte *>(s.data()), s.size_bytes() };
}

} // namespace bl

template<typename T>
struct std::hash<bl::span<T>>
{
    constexpr std::size_t operator()(const bl::span<T> &s) const noexcept
    {
        using CharT = std::decay_t<T>;
        if constexpr (std::is_integral_v<CharT>) {
            using SV = std::basic_string_view<CharT, std::char_traits<CharT>>;
            return std::hash<SV>{}(SV{ s.data(), s.size() });
        } else {
            return std::hash<std::span<T>>{}(
              std::span<T>{ s.data(), s.size() });
        }
    }
};

#endif // BL_SPAN_HPP
