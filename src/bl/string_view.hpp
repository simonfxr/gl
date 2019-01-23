#ifndef BL_STRING_VIEW_HPP
#define BL_STRING_VIEW_HPP

#include "bl/algorithm.hpp"
#include "bl/array_view.hpp"
#include "bl/string_fwd.hpp"
#include "bl/string_view_fwd.hpp"

namespace bl {

template<typename CharT>
struct basic_string_view
  : public comparable<basic_string_view<CharT>>
  , private array_view<const CharT>
{
    using base_t = array_view<const CharT>;

    using base_t::begin;
    using base_t::data;
    using base_t::empty;
    using base_t::end;
    using base_t::size;
    using base_t::operator[];
    using base_t::npos;
    using typename base_t::value_type;

    constexpr basic_string_view() = default;

    constexpr basic_string_view(const basic_string_view &) = default;
    constexpr basic_string_view(basic_string_view &&) = default;

    constexpr basic_string_view(const CharT *str)
      : basic_string_view(str, find_sentinel(str, CharT{}) - str)
    {}

    constexpr basic_string_view(const CharT *str, size_t n) : base_t(str, n) {}

    constexpr array_view<const CharT> array_view() const { return *this; }

    BL_constexpr size_t find(CharT ch, size_t pos = 0) const
    {
        if (empty() || pos >= size())
            return npos;
        auto p = ::bl::find(begin() + pos, end(), ch);
        return p == end() ? npos : p - begin();
    }

    BL_constexpr size_t rfind(CharT ch, size_t pos = npos) const
    {
        if (empty())
            return npos;
        if (pos >= size())
            pos = size();
        else
            ++pos;
        auto p = ::bl::rfind(begin(), begin() + pos, ch);
        return p ? p - begin() : npos;
    }

    BL_constexpr basic_string_view substr(size_t start, size_t n) const
    {
        auto v = base_t::slice(start, n);
        return { v.data(), v.size() };
    }

    BL_constexpr basic_string_view substr(size_t start) const
    {
        auto v = base_t::drop(start);
        return { v.data(), v.size() };
    }

#define DEF_STRING_BIN_OPS(ta, tb)                                             \
    template<typename Ch>                                                      \
    friend basic_string<Ch> operator+(ta a, tb b)                              \
    {                                                                          \
        basic_string<Ch> ret = a;                                              \
        ret += b;                                                              \
        return ret;                                                            \
    }                                                                          \
    template<typename Ch>                                                      \
    friend int compare(ta a, tb b)                                             \
    {                                                                          \
        return basic_string_view<Ch>(a).str_compare(basic_string_view<Ch>(b)); \
    }                                                                          \
    BL_DEF_REL_OPS_VIA(template<typename Ch> friend, ta, tb, compare(a, b))

    DEF_STRING_BIN_OPS(basic_string_view<Ch>, basic_string_view<Ch>)
    // DEF_STRING_BIN_OPS(basic_string_view<Ch>, const Ch *)
    // DEF_STRING_BIN_OPS(const Ch *, basic_string_view<Ch>)

    constexpr size_t hash() const noexcept
    {
        // really basic djb2 string hash
        size_t h = 5381;
        for (size_t i = 0; i < size(); ++i)
            h = h * 33 + size_t((*this)[i]);
        return h;
    }

#undef DEF_STRING_BIN_OPS

private:
    int str_compare(const basic_string_view &b) const
    {
        const auto &a = *this;
        auto n = a.size();
        auto m = b.size();
        auto nm = n <= m ? n : m;
        for (size_t i = 0; i < nm; ++i) {
            if (a[i] != b[i])
                return a[i] < b[i] ? -1 : 1;
        }

        return n < m ? -1 : n > m ? 1 : 0;
    }
};

basic_string_view(const char *)->basic_string_view<char>;
basic_string_view(char *)->basic_string_view<char>;
basic_string_view(const wchar_t *)->basic_string_view<wchar_t>;
basic_string_view(wchar_t *)->basic_string_view<wchar_t>;

using string_view = basic_string_view<char>;

using wstring_view = basic_string_view<wchar_t>;

template<typename T>
struct hash;

template<typename CharT>
struct hash<basic_string_view<CharT>>
{
    inline size_t operator()(basic_string_view<CharT> x) const
    {
        return x.hash();
    }
};

namespace literals {
inline string_view operator"" _sv(const char *s, unsigned long n)
{
    return string_view(s, n);
}

inline wstring_view operator"" _sv(const wchar_t *s, unsigned long n)
{
    return wstring_view(s, n);
}
} // namespace literals

} // namespace bl

#endif
