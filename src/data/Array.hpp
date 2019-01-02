#ifndef DATA_ARRAY_HPP
#define DATA_ARRAY_HPP

#include "defs.hpp"
#include "err/err.hpp"

#include <type_traits>

#define ARRAY_INITIALIZER(T)                                                   \
    {                                                                          \
        0, static_cast<T *>(nullptr)                                           \
    }

#define SHARE_ARRAY(ptr, s) ::make_shared_array(ptr, s)

template<typename T>
struct OwnedArray;

template<typename T>
struct Array
{
    size_t _size;
    T *_elems;

    constexpr Array(size_t s, T *es) : _size(s), _elems(es) {}

    Array(const OwnedArray<T> &) = delete;

    Array(OwnedArray<T> &&) = delete;

    constexpr Array(const Array &) = default;

    constexpr Array(Array &&) = default;

    constexpr size_t size() const { return _size; }

    constexpr T &operator[](size_t i) { return _elems[i]; }

    constexpr const T &operator[](size_t i) const { return _elems[i]; }

    constexpr T &at(size_t i) { return this->operator[](i); }

    constexpr const T &at(size_t i) const { return this->operator[](i); }

    Array &operator=(const Array &) = delete;
    Array &operator=(Array &&) = delete;

    constexpr T *begin() { return _elems; }
    constexpr T *end() { return _elems + _size; }

    constexpr const T *begin() const { return _elems; }
    constexpr const T *end() const { return _elems + _size; }

    constexpr const T *cbegin() const { return _elems; }
    constexpr const T *cend() const { return _elems + _size; }
};

template<typename T>
constexpr Array<T>
make_shared_array(T *elems, size_t s)
{
    Array<T> arr(s, elems);
    return arr;
}

template<typename T>
struct OwnedArray : public Array<T>
{

    explicit OwnedArray(size_t s = 0) : Array<T>(s, s == 0 ? nullptr : new T[s])
    {}

    template<typename U>
    OwnedArray(size_t s, const U *elems) : OwnedArray<T>(s)
    {
        for (size_t i = 0; i < s; ++i)
            this->at(i) = elems[i];
    }

    OwnedArray(const OwnedArray &arr) : OwnedArray(arr.size(), arr._elems) {}

    OwnedArray(const Array<T> &arr) : OwnedArray(arr.size(), arr._elems) {}

    OwnedArray(OwnedArray &&rhs)
      : Array<T>(std::exchange(rhs._size, 0),
                 std::exchange(rhs._elems, nullptr))
    {}

    ~OwnedArray() { delete[] this->_elems; }
};

#endif
