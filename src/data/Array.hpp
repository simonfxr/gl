#ifndef DATA_ARRAY_HPP
#define DATA_ARRAY_HPP

#include "defs.hpp"
#include "err/err.hpp"

#include <type_traits>

#define ARRAY_INITIALIZER(T)                                                   \
    {                                                                          \
        0, static_cast<T *>(nullptr)                                           \
    }

#define SHARE_ARRAY(s, ptr) make_shared_array(s, ptr)

template<typename T>
struct OwnedArray;

template<typename T>
struct Array
{
    defs::size _size;
    T *_elems;

    constexpr Array(defs::size s, T *es) : _size(s), _elems(es) {}

    Array(const OwnedArray<T> &) = delete;

    Array(OwnedArray<T> &&) = delete;

    constexpr Array(const Array &) = default;

    constexpr Array(Array &&) = default;

    constexpr defs::size size() const { return _size; }

    constexpr T &operator[](defs::index i) { return _elems[i]; }

    constexpr const T &operator[](defs::index i) const { return _elems[i]; }

    constexpr T &at(defs::index i) { return this->operator[](i); }

    constexpr const T &at(defs::index i) const { return this->operator[](i); }

    Array &operator=(const Array &) = delete;
    Array &operator=(Array &&) = delete;
};

template<typename T>
constexpr Array<T>
make_shared_array(defs::size s, T *elems)
{
    Array<T> arr(s, elems);
    return arr;
}

template<typename T>
struct OwnedArray : public Array<T>
{

    explicit OwnedArray(defs::size s = 0)
      : Array<T>(s, s == 0 ? nullptr : new T[UNSIZE(s)])
    {}

    template<typename U>
    OwnedArray(defs::size s, const U *elems) : OwnedArray<T>(s)
    {
        for (defs::index i = 0; i < s; ++i)
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
