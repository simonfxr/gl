#ifndef BL_STRING_HPP
#define BL_STRING_HPP

#include "bl/string_fwd.hpp"

#include "bl/compare.hpp"
#include "bl/string_view.hpp"
#include "bl/vector.hpp"

namespace bl {

template<typename CharT>
struct basic_string : private vector<CharT>
{
    using base_t = vector<CharT>;

    static inline constexpr size_t npos = size_t(-1);

    constexpr basic_string() = default;

    basic_string(size_t n) : base_t(n) {}

    basic_string(size_t n, CharT ch) : base_t(n, ch) {}

    basic_string(basic_string_view<CharT> view)
      : base_t(view.begin(), view.end())
    {}

    basic_string(const CharT *s, size_t n)
      : basic_string(basic_string_view<CharT>(s, n))
    {}

    basic_string(const CharT *s) : basic_string(basic_string_view<CharT>(s)) {}

    using typename base_t::const_iterator;
    using typename base_t::const_reference;
    using typename base_t::difference_type;
    using typename base_t::iterator;
    using typename base_t::reference;
    using typename base_t::size_type;
    using typename base_t::value_type;

    using base_t::begin;
    using base_t::data;
    using base_t::empty;
    using base_t::end;
    using base_t::size;
    using base_t::operator[];
    using base_t::beginp;
    using base_t::capacity;
    using base_t::clear;
    using base_t::emplace_back;
    using base_t::endp;
    using base_t::push_back;
    using base_t::reserve;
    using base_t::resize;

    size_t find(CharT ch, size_t pos = 0) const { return view().find(ch, pos); }

    size_t rfind(CharT ch, size_t pos = npos) const
    {
        return view().rfind(ch, pos);
    }

    const char *c_str() const
    {
        auto &self = const_cast<string &>(*this);
        if (self.capacity() == self.size())
            self.reserve(self.size() + 1);
        auto p = self.data();
        p[self.size()] = '\0';
        return p;
    }

    constexpr size_t hash() const noexcept { return view().hash(); }

    basic_string &operator+=(basic_string_view<CharT> b)
    {
        base_t::append(b.begin(), b.end());
        return *this;
    }

    basic_string_view<CharT> substr(size_t start, size_t n) const
    {
        return view().substr(start, n);
    }

    basic_string_view<CharT> substr(size_t start) const
    {
        return view().substr(start);
    }

    constexpr basic_string_view<CharT> view() const
    {
        return { data(), size() };
    }

    constexpr operator basic_string_view<CharT>() const { return view(); }

#define DEF_STRING_BIN_OPS(ta, tb)                                             \
    friend basic_string<CharT> operator+(ta a, tb b)                           \
    {                                                                          \
        basic_string<CharT> ret = a;                                           \
        ret += b;                                                              \
        return ret;                                                            \
    }                                                                          \
    friend int compare(ta a, tb b)                                             \
    {                                                                          \
        return compare(basic_string_view<CharT>(a),                            \
                       basic_string_view<CharT>(b));                           \
    }                                                                          \
    BL_DEF_REL_OPS_VIA(friend, ta, tb, compare(a, b))

    DEF_STRING_BIN_OPS(const basic_string<CharT> &, const basic_string<CharT> &)
    DEF_STRING_BIN_OPS(const basic_string<CharT> &, basic_string_view<CharT>)
    DEF_STRING_BIN_OPS(basic_string_view<CharT>, const basic_string<CharT> &)

#undef DEF_STRING_BIN_OP
};

template<typename T>
struct hash;

template<typename CharT>
struct hash<basic_string<CharT>>
{
    inline size_t operator()(basic_string_view<CharT> x) const
    {
        return x.hash();
    }
    inline size_t operator()(basic_string<CharT> x) const { return x.hash(); }
};

namespace literals {
inline string operator"" _s(const char *s, unsigned long n)
{
    return string(s, n);
}

inline wstring operator"" _s(const wchar_t *s, unsigned long n)
{
    return wstring(s, n);
}
} // namespace literals
} // namespace bl

#endif
