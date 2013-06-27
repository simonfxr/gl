#ifndef DATA_ARRAY_HPP
#define DATA_ARRAY_HPP

#include "defs.hpp"

template <typename T>
struct Array {
protected:
    defs::size _size;
    T * _elems;

public:

    Array() : _size(0), _elems(0) {}     
    
    Array(defs::size s):
        _size(ASSERT_SIZE(s)),
        _elems(new T[UNSIZE(s)])
        {}
    
    Array(const Array<T>& a) :
        _size(a._size), _elems(a._size == 0 ? 0 : new T[UNSIZE(a._size)])
    {
        for (defs::index i = 0; i < _size; ++i)
            (*this)[i] = a[i];
    }
    
    ~Array() { if (_elems) delete[] _elems; }
    
    defs::size size() const { return _size; }
    
    T& operator[](defs::index i) { return _elems[i]; }
    
    const T& operator[](defs::index i) const { return _elems[i]; }
    
    T& at(defs::index i) { return this->operator[](i); }
    
    const T& at(defs::index i) const { return this->operator[](i); }

    void unsafeResize(defs::size n) {
        if (n != _size) {
            if (_elems) delete _elems;
            _size = n;
            _elems = new T[UNSIZE(n)];
        }
    }

protected:

    Array(defs::size s, T *elems) :
        _size(s), _elems(elems) {}
};

#endif
