#ifndef BL_ARRAY_VIEW_HPP
#define BL_ARRAY_VIEW_HPP

#include "bl/array_view_fwd.hpp"

#include "bl/compare.hpp"
#include "bl/type_traits.hpp"

namespace bl {

template<typename T>
struct array_view : public comparable<array_view<T>>
{
    constexpr array_view() = default;

    template<typename U = T>
    constexpr array_view(U *es, size_t s) : _size(s), _elems(es)
    {}

    constexpr array_view(const array_view &v) = default;

    template<typename U>
    constexpr array_view(const array_view<U> &v)
      : _size(v.size()), _elems(v.data())
    {}

    constexpr array_view(array_view &&) = default;

    constexpr bool empty() const { return _size == 0; }

    constexpr size_t size() const { return _size; }

    constexpr T &operator[](size_t i) const { return _elems[i]; }

    array_view &operator=(const array_view &) = delete;
    array_view &operator=(array_view &&) = delete;

    constexpr T *data() const { return _elems; }

    constexpr array_view<const T> as_const() const { return { _elems, _size }; }

    constexpr T *begin() const { return _elems; }
    constexpr T *end() const { return _elems + _size; }

    constexpr array_view<T> slice(size_t start, size_t n) const
    {
        // ASSERT(start < size());
        // ASSERT(start + n <= size());
        return { _elems + start, n };
    }

    constexpr array_view<T> drop(size_t n) const
    {
        // ASSERT(n <= size());
        return { _elems + n, _size - n };
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
    size_t _size{};
    T *_elems{};
};

template<typename T>
array_view(const T *, size_t n)->array_view<const T *>;

template<typename T>
array_view(T *, size_t n)->array_view<T *>;

} // namespace bl

#endif
