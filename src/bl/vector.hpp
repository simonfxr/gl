#ifndef BL_VECTOR_HPP
#define BL_VECTOR_HPP

#include "bl/array_view.hpp"
#include "bl/new.hpp"
#include "bl/vector_fwd.hpp"

#include <bits/move.h>

namespace bl {

template<typename T>
struct vector : public comparable<vector<T>>
{
    constexpr vector() = default;

    vector(size_t n)
      : _data(n > 0 ? new_bare_array<T>(n) : nullptr), _count(n), _capa(n)
    {}

    vector(size_t n, const T &init)
      : _data(n > 0 ? new_bare_array<T>(n, init) : nullptr), _count(n), _capa(n)
    {}

    template<typename U>
    constexpr vector(const vector<U> &v) : vector(v.begin(), v.end())
    {}

    template<typename U = T>
    constexpr vector(U *first, U *last)
    {
        auto n = last - first;
        if (n > 0) {
            _data = copy_bare_array<T>(first, n);
            _count = n;
            _capa = n;
        }
    }

    template<typename Iter>
    constexpr vector(Iter first, Iter last)
    {
        append(first, last);
    }

    template<typename U = T>
    constexpr vector(array_view<U> v) : vector(v.begin(), v.end())
    {}

    template<typename U>
    constexpr vector(vector &&v)
      : _data(exchange(v._data, nullptr))
      , _count(exchange(v._count), 0)
      , _capa(exchange(v._capa), 0)
    {}

    template<typename U = T>
    vector &operator=(const vector<U> &)
    {
        // TODO: implement
        return *this;
    }

    template<typename U>
    vector &operator=(vector<U> &&) noexcept
    {
        // TODO: implement
        return *this;
    }

    constexpr array_view<const T> view() const { return { *this }; }
    constexpr array_view<T> view() { return { *this }; }

    constexpr operator array_view<const T>() const { return view(); }
    constexpr operator array_view<T>() { return view(); }

    template<typename U = T>
    vector &operator+=(array_view<const U>)
    {
        // TODO: implement
        return *this;
    }

    template<typename U>
    constexpr friend int compare(const vector<T> &a, const vector<U> &b)
    {
        return a.view().compare(b.view());
    }

    void push_front(const T &value) { push_front(T(value)); }

    void push_front(T &&)
    {
        // TODO: implement
    }

    void pop_back()
    {
        // TODO: implement
    }

    template<typename Iter>
    void append(Iter first, Iter last)
    {
        for (; first != last; ++first)
            push_back(*first);
        return *this;
    }

    template<typename U>
    void append(array_view<const U> arr)
    {
        append(arr.begin(), arr.end());
    }

    const T *data() const { return _data; }
    T *data() { return _data; }

    const T *begin() const { return data(); }
    T *begin() { return data(); }

    const T *end() const { return data() + _count; }
    T *end() { return data() + _count; }

    bool empty() const { return _count == 0; }

    size_t size() const { return _count; }
    size_t capacity() const { return _capa; }

    const T &operator[](size_t i) const { return _data[i]; }
    T &operator[](size_t i) { return _data[i]; }

    const T &back() const { return _data[_count - 1]; }
    T &back() { return _data[_count - 1]; }

    const T &front() const { return _data[0]; }
    T &front() { return _data[0]; }

    void clear()
    {
        // TODO: implement
    }

    void reserve(size_t)
    {
        // TODO: implement
    }

    void resize(size_t)
    {
        // TODO: implement
    }

    void resize(size_t, const T &)
    {
        // TODO: implement
    }

    void push_back(const T &)
    {
        // TODO: implement
    }

    void push_back(T &&)
    {
        // TODO: implement
    }

    template<typename... Args>
    T &emplace_back(Args &&...)
    {
        // TODO: implement
        return front();
    }

    const T *erase(const T *p)
    {
        // TODO: implement
        return p;
    }

    T *erase(T *p)
    {
        // TODO: implement
        return p;
    }

private:
    T *_data{};
    uint32_t _count = 0;
    uint32_t _capa = 0;
};

template<typename T>
inline constexpr vector<T>
make_vector()
{
    return {};
}

template<typename T, typename Arg, typename... Args>
inline constexpr vector<T>
make_vector(Arg &&arg, Args &&... args)
{
    T elems[] = { static_cast<T>(std::forward<Arg>(arg)),
                  static_cast<T>(std::forward<Args>(args))... };
    return vector<T>(elems, elems + sizeof...(args));
}

} // namespace bl

#endif
