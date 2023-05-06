#ifndef BL_CSTRING_HPP
#define BL_CSTRING_HPP

#include <bl/optional.hpp>
#include <bl/string.hpp>

namespace bl {

template<usize SmallBuffer = 96,
         typename CharT = char,
         typename Allocator = std::allocator<CharT>>
struct basic_cstring_view
{
    static_assert(sizeof(CharT) == 1);
    static_assert(SmallBuffer > 0);
    basic_cstring_view() = delete;
    basic_cstring_view(const basic_cstring_view &) = delete;
    basic_cstring_view(basic_cstring_view &&) = delete;
    basic_cstring_view &operator=(const basic_cstring_view &) = delete;
    basic_cstring_view &operator=(basic_cstring_view &&) = delete;

    constexpr /* implicit */ basic_cstring_view(
      const basic_string<CharT, Allocator> &s)
      : _largep(&s)
    {}
    constexpr /* implicit */ basic_cstring_view(basic_string_view<CharT> sv)
    {
        if (sv.size() < SmallBuffer) {
            *std::copy(sv.begin(), sv.end(), &_small[0]) =
              static_cast<CharT>(0);
        } else {
            _large = { basic_string<CharT, Allocator>(sv) };
            _largep = &*_large;
        }
    }
    ~basic_cstring_view() = default;

    const CharT *c_str() const noexcept
    {
        return _largep ? _largep->c_str() : &_small[0];
    }

    /* implicit */ operator const CharT *() const noexcept { return c_str(); }

private:
    bl::basic_string<CharT, Allocator> *_largep{};
    bl::optional<bl::basic_string<CharT, Allocator>> _large;
    CharT _small[SmallBuffer];
};

using cstring_view = basic_cstring_view<>;

} // namespace bl

#endif // BL_CSTRING_HPP
