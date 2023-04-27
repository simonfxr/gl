#ifndef BL_CSTRING_HPP
#define BL_CSTRING_HPP

#include <bl/new.hpp>
#include <bl/string.hpp>

#include <array>

namespace bl {

template<size_t SmallBuffer = 256, typename CharT = char>
struct basic_cstring_view
{
    static_assert(sizeof(CharT) == 1);
    basic_cstring_view() = delete;
    basic_cstring_view(const basic_cstring_view &) = delete;
    basic_cstring_view(basic_cstring_view &&) = delete;
    basic_cstring_view &operator=(const basic_cstring_view &) = delete;
    basic_cstring_view &operator=(basic_cstring_view &&) = delete;
    /* implicit */ basic_cstring_view(const basic_string<CharT> &s) : large(&s)
    {}
    /* implicit */ basic_cstring_view(const basic_string_view<CharT> &sv)
    {
        if (sv.size() < SmallBuffer) {
            *std::copy(sv.begin(), sv.end(), small.begin()) =
              static_cast<CharT>(0);
        } else {
            large = new_obj<basic_string<CharT>>(sv);
            dealloc = true;
        }
    }
    ~basic_cstring_view()
    {
        if (dealloc)
            delete_obj(large);
    }

    const CharT *c_str() const noexcept
    {
        return large ? large->c_str() : small.data();
    }

    /* implicit */ operator const CharT *() const noexcept { return c_str(); }

private:
    std::array<CharT, SmallBuffer> small;
    const basic_string<CharT> *large;
    bool dealloc = false;
};

using cstring_view = basic_cstring_view<>;

} // namespace bl

#endif // BL_CSTRING_HPP
