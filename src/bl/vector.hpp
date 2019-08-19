#ifndef BL_VECTOR_HPP
#define BL_VECTOR_HPP

#include "bl/algorithm.hpp"
#include "bl/array_view.hpp"
#include "bl/core.hpp"
#include "bl/debug.hpp"
#include "bl/iterator.hpp"
#include "bl/memory.hpp"
#include "bl/new.hpp"
#include "bl/swap.hpp"
#include "bl/vector_fwd.hpp"

// #define BL_STD_VECTOR 1

#ifdef BL_STD_VECTOR
#    include <vector>
#endif

namespace bl {

#ifdef BL_STD_VECTOR

template<typename T>
struct vector : private std::vector<T>
{

    using base_t = std::vector<T>;
    using std::vector<T>::vector;

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
        bl::move(bl::move(first), bl::move(last), std::back_inserter(*this));
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
        base_t::insert(begin(), bl::move(x));
        return front();
    }
};

#else

template<typename T>
struct vector : public comparable<vector<T>>
{
    static_assert(bl::is_same_v<bl::remove_cv_t<T>, T>,
                  "T cannot be cv qualified");

    using value_type = T;
    using reference = T &;
    using const_reference = const T &;
    using iterator = T *;
    using const_iterator = const T *;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    constexpr vector() = default;

    explicit vector(size_t n) { resize(n); }

    vector(size_t n, const T &init) { resize(n, init); }

    vector(const vector &v) : vector(v.begin(), v.end()) {}

    template<typename U>
    vector(const vector<U> &v) : vector(v.begin(), v.end())
    {}

    vector(vector &&v) noexcept
      : _data(exchange(v._data, nullptr))
      , _size(exchange(v._size, 0))
      , _capa(exchange(v._capa, 0))
    {}

    template<typename U>
    vector(vector<U> &&v)
    {
        assign(move_tag_t{}, v.begin(), v.end());
    }

    template<typename Iter>
    vector(Iter first, Iter last)
    {
        append(copy_tag_t{}, first, last);
    }

    template<typename U = T>
    constexpr explicit vector(array_view<U> v) : vector(v.begin(), v.end())
    {}

    ~vector() { reset(); }

    template<typename AssignTag, typename U>
    void assign(AssignTag assign_tag, const U *first, const U *last)
    {
        if (first == _data)
            return;
        BL_ASSERT(last >= first);
        auto n = size_t(last - first);

        if (_capa < n) {
            // _capa < n => this is not a self assignment
            reset();
            append(assign_tag, first, last);
            return;
        }

        auto common = min(n, size());
        auto cpy_end = ::bl::assign(assign_tag, first, first + common, begin());
        if (n <= size()) {
            destroy(cpy_end, end());
            _size = n;
        } else {
            uninitialized_assign(
              assign_tag, first + size(), first + (n - size()), end());
            _size = n;
        }
        check();
    }

    vector &operator=(const vector &x)
    {
        assign(copy_tag_t{}, x.begin(), x.end());
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
        _size = exchange(rhs._size, 0);
        _capa = exchange(rhs._capa, 0);
        check();
        return *this;
    }

    template<typename U>
    vector &operator=(vector<U> &&rhs)
    {
        return *this = vector<T>(bl::move(rhs));
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

    template<typename AssignTag, typename Iter>
    void append(AssignTag assign_tag, Iter first, Iter last)
    {
        if constexpr (is_random_access_iterator<Iter>) {
            BL_ASSERT(last >= first);
            size_t n = last - first;
            reserve(size() + n);
            auto p =
              uninitialized_assign(assign_tag, first, last, _data + _size);
            BL_ASSERT(size_t(p - _data) == _size + n);
            UNUSED(p);
            _size += n;
        } else {
            for (; first != last; ++first)
                push_back(assign_tag, *first);
        }
        check();
    }

    template<typename Iter>
    void append(Iter first, Iter last)
    {
        append(copy_tag_t{}, bl::move(first), bl::move(last));
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

    const T *end() const { return data() + _size; }
    T *end() { return data() + _size; }

    constexpr const T *endp() const { return data() + size(); }
    constexpr T *endp() { return data() + size(); }

    constexpr const T *beginp() const { return data(); }
    constexpr T *beginp() { return data(); }

    bool empty() const { return _size == 0; }

    size_t size() const { return _size; }
    size_t capacity() const { return _capa; }

    const T &operator[](size_t i) const { return _data[i]; }
    T &operator[](size_t i) { return _data[i]; }

    const T &back() const { return _data[_size - 1]; }
    T &back() { return _data[_size - 1]; }

    const T &front() const { return _data[0]; }
    T &front() { return _data[0]; }

    void clear()
    {
        destroy(begin(), end());
        _size = 0;
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
            BL_ASSERT(_size < n);
            uninitialized_default_construct(end(), begin() + n);
            _size = n;
        }
        check();
    }

    void resize(size_t n, const T &x)
    {
        if (_resize(n)) {
            BL_ASSERT(_size < n);
            uninitialized_fill(end(), begin() + n, x);
            _size = n;
        }
        check();
    }

    const T *erase(const T *p) { return erase(const_cast<T *>(p)); }

    T *erase(T *p)
    {
        BL_ASSERT(begin() <= p && p < end());
        if (p + 1 != end())
            move(p + 1, end(), p);
        --_size;
        check();
        return p;
    }

    template<typename AssignTag, typename U>
    T &push_back(AssignTag assign_tag, U &&x)
    {
        ensure_capa();
        auto &y = initialize(assign_tag, _data + _size, bl::forward<U>(x));
        ++_size;
        return y;
    }

    T &push_back(const T &x) { return push_back(copy_tag_t{}, x); }

    T &push_back(T &&x) { return push_back(move_tag_t{}, bl::move(x)); }

    template<typename... Args>
    T &emplace_back(Args &&... args)
    {
        ensure_capa();
        auto y = new (_data + _size) T(bl::forward<Args>(args)...);
        ++_size;
        return *y;
    }

    template<typename AssignTag, typename U>
    T &push_front(AssignTag assign_tag, U &&x)
    {
        shift();
        return initialize(assign_tag, _data, bl::forward<U>(x));
    }

    T &push_front(const T &x) { return push_front(copy_tag_t{}, x); }

    T &push_front(T &&x) { return push_front(move_tag_t{}, bl::move(x)); }

    void pop_back()
    {
        BL_ASSERT(_size > 0);
        destroy_at(_data + (_size - 1));
        --_size;
        check();
    }

private:
    void check_iterator(const T *p)
    {
        BL_ASSERT(begin() <= p && p <= end());
        UNUSED(p);
        check();
    }

    bool _resize(size_t n)
    {
        if (n <= _size) {
            destroy(begin() + n, end());
            _size = n;
            check();
            return false;
        }

        if (n > _capa) {
            auto buf = new_uninitialized_bare_array<T>(n);
            auto new_end = uninitialized_destructive_move(begin(), end(), buf);
            BL_ASSERT(new_end - buf == _size);
            UNUSED(new_end);
            swap(_data, buf);
            delete_uninitialized_bare_array(buf, _size);
            _capa = n;
            check();
        }

        return true;
    }

    void shift()
    {
        reserve(size() + 1);
        if (!_size)
            return;
        new (end()) T(bl::move(*(end() - 1)));
        move(begin(), end() - 1, begin() + 1);
        ++_size;
        check();
    }

    void ensure_capa()
    {
        if_unlikely(_size == _capa)
          reserve(_size == 0 ? min_capacity : _size * 2);
        check();
    }

    void reset()
    {
        if (!_data) {
            BL_ASSERT(_size == 0 && _capa == 0);
            check();
            return;
        }
        BL_ASSERT(_capa > 0);
        destroy(begin(), end());
        delete_uninitialized_bare_array(_data, _capa);
        _data = nullptr;
        _size = _capa = 0;
        check();
    }

    T *_data{};
    uint32_t _size = 0;
    uint32_t _capa = 0;

    void check()
    {
        BL_ASSERT((_data != nullptr) == (_capa != 0));
        BL_ASSERT(_size <= _capa);
        BL_ASSERT(_capa < 10000);
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
    T elems[] = { static_cast<T>(bl::forward<Arg>(arg)),
                  static_cast<T>(bl::forward<Args>(args))... };
    vector<T> v;
    v.assign(move_tag_t{}, elems, elems + ARRAY_LENGTH(elems));
    return v;
}

} // namespace bl

#endif
