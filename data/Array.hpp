#ifndef DATA_ARRAY_HPP
#define DATA_ARRAY_HPP

#include "defs.h"
#include "error/error.hpp"

template <typename T>
struct Array {
private:
    const uint32 _size;
    T * const _elems;
    bool owning_elems;

public:
    // disambiguate constructor
    enum Ownership {
        Shared,
        Owned
    };
    
    Array(uint32 s, Ownership own = Shared) :
        _size(s), _elems(new T[s]), owning_elems(own == Owned) {}
    
    Array(T *els, uint32 s, Ownership own = Shared) :
        _size(s), _elems(els), owning_elems(own == Owned) {}
    
    Array(const Array<T>& a) :
        _size(a._size), _elems(a._elems), owning_elems(false) {
        ASSERT_MSG(!a.owning_elems, "cannot copy array with ownership");
    }
    
    ~Array() {
        if (owning_elems) delete[] _elems;
    }
    
    uint32 size() const { return _size; }
    
    T& operator[](uint32 i) { return _elems[i]; }
    
    const T& operator[](uint32 i) const { return _elems[i]; }
    
    T& get(uint32 i) { return this->operator[](i); }
    
    const T& get(uint32 i) const { return this->operator[](i); }

    void setDelete(bool do_delete = true) { owning_elems = do_delete; }

    Array<T> clone() const {
        T *elems = new T[_size];
        for (uint32 i = 0; i < _size; ++i)
            elems[i] = _elems[i];
        return Array<T>(elems, _size, Owned);
    }
};


#define DEFINE_ARRAY(name, type, ...)                                   \
    static type CONCAT(_array_data_, name)[] = { __VA_ARGS__ };                \
    Array<type> name(CONCAT(_array_data_, name), ARRAY_LENGTH(CONCAT(_array_data_, name)))

#define DEFINE_CONST_ARRAY(name, type, ...)                             \
    static type CONCAT(_array_data_, name)[] = { __VA_ARGS__ };         \
    const Array<type> name(CONCAT(_array_data_, name), ARRAY_LENGTH(CONCAT(_array_data_, name)))

// FIXME: can this be made portable (maybe lambda's) ?
// #ifdef GNU_EXTENSIONS 
#define CONST_ARRAY(type, ...) ({                                       \
            static type CONCAT(_array_data_, _anon_arr)[] = { __VA_ARGS__ }; \
            static const Array<type> _anon_arr(CONCAT(_array_data_, _anon_arr), ARRAY_LENGTH(CONCAT(_array_data_, _anon_arr))); \
            _anon_arr;                                                  \
        })
// #endif

#endif
