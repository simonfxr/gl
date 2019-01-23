#ifndef BL_VECTOR_HPP
#define BL_VECTOR_HPP

#include "bl/algorithm.hpp"
#include "bl/array_view.hpp"
#include "bl/core.hpp"
#include "bl/iterator.hpp"
#include "bl/memory.hpp"
#include "bl/new.hpp"
#include "bl/swap.hpp"
#include "bl/vector_fwd.hpp"
#include "err/err.hpp"

#define BL_STD_VECTOR 1

#ifdef BL_STD_VECTOR
#    include <vector>
#endif

#ifndef NDEBUG
#    include "bl/string_view.hpp"
#endif

namespace bl {

#ifdef BL_STD_VECTOR

template<typename T>
struct vector : private std::vector<T>
{

    using base_t = std::vector<T>;
    using std::vector<T>::vector;

    template<typename U = T>
    constexpr vector(array_view<U> v) : vector(v.begin(), v.end())
    {}

    using base_t::begin;
    using base_t::clear;
    using base_t::empty;
    using base_t::end;
    using base_t::size;
    using base_t::operator[];
    using base_t::back;
    using base_t::capacity;
    using base_t::data;
    using base_t::emplace_back;
    using base_t::erase;
    using base_t::front;
    using base_t::pop_back;
    using base_t::push_back;
    using base_t::reserve;
    using base_t::resize;
    using typename base_t::reference;
    using typename base_t::value_type;

    template<typename Iter>
    void append(Iter first, Iter last)
    {
        base_t::insert(end(), first, last);
    }

    template<typename U>
    void append(array_view<U> arr)
    {
        append(arr.begin(), arr.end());
    }

    constexpr array_view<const T> view() const { return { data(), size() }; }
    constexpr array_view<T> view() { return { data(), size() }; }

    constexpr operator array_view<const T>() const { return view(); }
    constexpr operator array_view<T>() { return view(); }

    template<typename Iter>
    void move_assign(Iter first, Iter last)
    {
        std::move(std::move(first), std::move(last), std::back_inserter(*this));
    }

    constexpr const T *endp() const { return data() + size(); }
    constexpr T *endp() { return data() + size(); }

    constexpr const T *beginp() const { return data(); }
    constexpr T *beginp() { return data(); }

    T &push_front(const T &x)
    {
        base_t::insert(begin(), x);
        return front();
    }

    T &push_front(T &&x)
    {
        base_t::insert(begin(), std::move(x));
        return front();
    }
};

#else

template<typename T>
struct vector : public comparable<vector<T>>
{
    constexpr vector() = default;

    vector(size_t n) { resize(n); }

    vector(size_t n, const T &init) { resize(n, init); }

    vector(const vector &v) : vector(v.begin(), v.end()) {}

    template<typename U>
    vector(const vector<U> &v) : vector(v.begin(), v.end())
    {}

    vector(vector &&v) noexcept
      : _data(exchange(v._data, nullptr))
      , _count(exchange(v._count, 0))
      , _capa(exchange(v._capa, 0))
    {}

    template<typename U>
    vector(vector<U> &&v)
    {
        move_assign(v.begin(), v.end());
    }

    template<typename Iter>
    vector(Iter first, Iter last)
    {
        append(first, last);
    }

    template<typename U = T>
    constexpr vector(array_view<U> v) : vector(v.begin(), v.end())
    {}

    ~vector() { reset(); }

    template<typename U>
    void assign(const U *first, const U *last)
    {
        if (first == _data)
            return;
        ASSERT(last >= first);
        auto n = size_t(last - first);

        if (_capa < n) {
            reset();
            append(first, last);
            return;
        }

        if constexpr (std::is_trivially_constructible_v<T, const U &> &&
                      std::is_trivially_assignable_v<T, const U &> &&
                      std::is_trivially_destructible_v<T>) {
            copy(first, last, begin());
            _count = n;
        } else {
            auto common = min(n, size());
            auto cpy_end = copy(first, first + common, begin());
            if (n <= size()) {
                destroy(cpy_end, end());
                _count = n;
            } else {
                uninitialized_copy(first + size(), first + (n - size()), end());
                _count = n;
            }
        }
        check();
    }

    template<typename U>
    void move_assign(U *first, U *last)
    {
        if (first == _data)
            return;
        ASSERT(last >= first);
        auto n = size_t(last - first);

        if (_capa < n) {
            reset();
            append(first, last);
            return;
        }

        if constexpr (std::is_trivially_constructible_v<T, U &&> &&
                      std::is_trivially_assignable_v<T, U &&> &&
                      std::is_trivially_destructible_v<T>) {
            move(first, last, begin());
            _count = n;
        } else {
            auto common = min(n, size());
            auto cpy_end = move(first, first + common, begin());
            if (n <= size()) {
                destroy(cpy_end, end());
                _count = n;
            } else {
                uninitialized_move(first + size(), first + (n - size()), end());
                _count = n;
            }
        }
        check();
    }

    vector &operator=(const vector &x)
    {
        assign(x.begin(), x.end());
        return *this;
    }

    template<typename U = T>
    vector &operator=(array_view<U> av)
    {
        assign(av.begin(), av.end());
        return *this;
    }

    template<typename U>
    vector &operator=(const vector<U> &x)
    {
        return assign(x.begin(), x.end());
    }

    vector &operator=(vector &&rhs) noexcept
    {
        reset();
        _data = exchange(rhs._data, nullptr);
        _count = exchange(rhs._count, 0);
        _capa = exchange(rhs._capa, 0);
        check();
        return *this;
    }

    template<typename U>
    vector &operator=(vector<U> &&rhs)
    {
        move_assign(rhs.begin(), rhs.end());
        return *this;
    }

    constexpr array_view<const T> view() const { return { data(), size() }; }
    constexpr array_view<T> view() { return { data(), size() }; }

    constexpr operator array_view<const T>() const { return view(); }
    constexpr operator array_view<T>() { return view(); }

    template<typename U>
    constexpr friend int compare(const vector<T> &a, const vector<U> &b)
    {
        return a.view().compare(b.view());
    }

    template<typename Iter>
    void append(Iter first, Iter last)
    {
        if constexpr (is_random_access_iterator<Iter>) {
            ASSERT(last >= first);
            size_t n = last - first;
            reserve(size() + n);
            auto p = uninitialized_copy(first, last, _data + _count);
            ASSERT(size_t(p - _data) == _count + n);
            _count += n;
        } else {
            for (; first != last; ++first)
                push_back(*first);
        }
        check();
    }

    template<typename U>
    void append(array_view<U> arr)
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
        destroy(begin(), end());
        _count = 0;
        check();
    }

    void reserve(size_t n)
    {
        if (n > _capa) {
            auto buf = new_uninitialized_bare_array<T>(n);
            uninitialized_destructive_move(begin(), end(), buf);
            delete_uninitialized_bare_array(_data, _capa);
            _data = buf;
            _capa = n;
        }
        check();
    }

    void resize(size_t n)
    {
        if (_resize(n)) {
            ASSERT(_count < n);
            uninitialized_default_construct(end(), begin() + n);
            _count = n;
        }
        check();
    }

    void resize(size_t n, const T &x)
    {
        if (_resize(n)) {
            ASSERT(_count < n);
            uninitialized_fill(end(), begin() + n, x);
            _count = n;
        }
        check();
    }

    const T *erase(const T *p) { return erase(const_cast<T *>(p)); }

    T *erase(T *p)
    {
        ASSERT(begin() <= p && p < end());
        if (p + 1 != end())
            move(p + 1, end(), p);
        --_count;
        check();
        return p;
    }

    T &push_back(const T &x)
    {
        ensure_capa();
        auto y = new (_data + _count) T(x);
        ++_count;
        return *y;
    }

    T &push_back(T &&x)
    {
        ensure_capa();
        auto y = new (_data + _count) T(std::move(x));
        ++_count;
        return *y;
    }

    template<typename... Args>
    T &emplace_back(Args &&... args)
    {
        ensure_capa();
        auto y = new (_data + _count) T(std::forward<Args>(args)...);
        ++_count;
        return *y;
    }

    T &push_front(const T &x)
    {
        shift();
        auto y = new (_data) T(x);
        return *y;
    }

    T &push_front(T &&x)
    {
        shift();
        auto y = new (_data) T(std::move(x));
        return *y;
    }

    void pop_back()
    {
        ASSERT(_count > 0);
        destroy_at(_data + (_count - 1));
        --_count;
        check();
    }

private:
    void check_iterator(const T *p)
    {
        ASSERT(begin() <= p && p <= end());
        UNUSED(p);
        check();
    }

    bool _resize(size_t n)
    {
        if (n <= _count) {
            destroy(begin() + n, end());
            _count = n;
            check();
            return false;
        }

        if (n > _capa) {
            auto buf = new_uninitialized_bare_array<T>(n);
            auto new_end = uninitialized_destructive_move(begin(), end(), buf);
            ASSERT(new_end - buf == _count);
            UNUSED(new_end);
            swap(_data, buf);
            delete_uninitialized_bare_array(buf, _count);
            _capa = n;
            check();
        }

        return true;
    }

    void shift()
    {
        reserve(size() + 1);
        if constexpr (std::is_trivially_move_constructible_v<T> &&
                      std::is_trivially_move_assignable_v<T>) {
            move(begin(), end(), begin() + 1);
        } else {
            if (!_count)
                return;
            new (end()) T(std::move(*(end() - 1)));
            move(begin(), end() - 1, begin() + 1);
        }
        ++_count;
        check();
    }

    void ensure_capa()
    {
        if (unlikely(_count == _capa))
            reserve(_count == 0 ? min_capacity : _count * 2);
        check();
    }

    void reset()
    {
        if (!_data) {
            ASSERT(_count == 0 && _capa == 0);
            check();
            return;
        }
        ASSERT(_capa > 0);
        destroy(begin(), end());
        delete_uninitialized_bare_array(_data, _capa);
        _data = nullptr;
        _count = _capa = 0;
        check();
    }

    T *_data{};
    uint32_t _count = 0;
    uint32_t _capa = 0;

    void check()
    {
        ASSERT((_data != nullptr) == (_capa != 0));
        ASSERT(_count <= _capa);
        ASSERT(_capa < 10000);
    }

    static inline constexpr size_t min_capacity = sizeof(T) > 32
                                                    ? 1
                                                    : 32 / sizeof(T);
};

#endif

template<typename T>
inline constexpr vector<T>
make_vector()
{
    return {};
}

template<typename T, typename Arg, typename... Args>
inline vector<T>
make_vector(Arg &&arg, Args &&... args)
{
    T elems[] = { static_cast<T>(std::forward<Arg>(arg)),
                  static_cast<T>(std::forward<Args>(args))... };
    vector<T> v;
    v.move_assign(elems, elems + ARRAY_LENGTH(elems));
    ASSERT(v.size() == 1 + sizeof...(args));
    return v;
}

} // namespace bl

#endif
