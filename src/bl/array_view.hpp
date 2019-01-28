#ifndef BL_ARRAY_VIEW_HPP
#define BL_ARRAY_VIEW_HPP

#include "bl/array_view_fwd.hpp"

#include "bl/compare.hpp"
#include "bl/debug.hpp"
#include "bl/type_traits.hpp"

namespace bl {

template<typename T>
struct array_view : public comparable<array_view<T>>
{
    using value_type = T;
    using reference = T &;
    using const_reference = const T &;
    using iterator = T *;
    using const_iterator = const T *;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    static inline constexpr size_t npos = size_t(-1);

    BL_inline constexpr array_view() noexcept = default;

    template<typename U = T>
    BL_inline constexpr array_view(U *d, size_t s) noexcept : _data(d), _size(s)
    {}

    BL_inline constexpr array_view(const array_view &v) noexcept = default;

    template<typename U>
    BL_inline constexpr array_view(const array_view<U> &v) noexcept
      : array_view(v.data(), v.size())
    {}

    BL_inline constexpr array_view(array_view &&) noexcept = default;

    BL_inline constexpr bool empty() const noexcept { return _size == 0; }

    BL_inline constexpr size_t size() const noexcept { return _size; }

    BL_inline constexpr T &operator[](size_t i) const { return _data[i]; }

    BL_inline array_view &operator=(const array_view &) noexcept = default;

    template<typename U>
    BL_inline array_view &operator=(const array_view<U> &av) noexcept
    {
        _data = av.data();
        _size = av.size();
    }

    BL_inline constexpr T *data() const noexcept { return _data; }

    BL_inline constexpr array_view<const T> as_const() const noexcept
    {
        return { data(), size() };
    }

    BL_inline constexpr T *begin() const noexcept { return data(); }
    BL_inline constexpr T *end() const noexcept { return data() + size(); }

    BL_inline constexpr T *beginp() const noexcept { return begin(); }
    BL_inline constexpr T *endp() const noexcept { return end(); }

    BL_inline BL_constexpr T &front() const noexcept
    {
        BL_ASSERT(!empty());
        return *begin();
    }

    BL_inline BL_constexpr T &back() const noexcept
    {
        BL_ASSERT(!empty());
        return endp()[-1];
    }

    BL_inline constexpr array_view<T> remove_prefix(size_t n) const noexcept
    {
        if (n <= size())
            return { data() + n, size() - n };
        else
            return {};
    }

    BL_inline constexpr array_view<T> remove_suffix(size_t n) const noexcept
    {
        if (n <= size())
            return { data(), size() - n };
        else
            return {};
    }

    BL_inline constexpr array_view<T> subspan(size_t offset) const noexcept
    {
        return remove_prefix(offset);
    }

    BL_inline BL_constexpr array_view<T> subspan(size_t offset, size_t n) const
      noexcept
    {
        // FIXME: offset + n might overflow
        if (offset + n <= size())
            return { data() + offset, n };
        else if (offset <= size())
            return { data() + offset, size() - offset };
        else
            return {};
    }

    template<typename U>
    constexpr int compare(array_view<const U> b) const
    {
        const auto &a = *this;
        auto n = a.size();
        auto m = b.size();
        auto nm = n <= m ? n : m;
        for (size_t i = 0; i < nm; ++i) {
            if (a[i] != b[i]) {
                using ::bl::compare;
                return compare(a[i], b[i]);
            }
        }
        return n < m ? -1 : n > m ? 1 : 0;
    }

private:
    T *_data;
    size_t _size;
};

template<typename T>
array_view(const T *, size_t n)->array_view<const T *>;

template<typename T>
array_view(T *, size_t n)->array_view<T *>;

} // namespace bl

#endif
