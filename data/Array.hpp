#ifndef DATA_ARRAY_HPP
#define DATA_ARRAY_HPP

#include "defs.h"
#include "error/error.hpp"

#include <iostream>

#define VAL ((void *) 0xDEADBEEFDEADBEEFul)

template <typename T>
struct Array;

extern void register_arr(const Array<int> *arr);
extern void unregister_arr(const Array<int> *arr);
extern void check_arrs();

template <typename T>
struct Array {
public:
    void * magic1;
    const uint32 _size;
    T * const _elems;
    bool owning_elems;
    void * magic2;
    
public:
    Array(T *els, uint32 s, bool owns = false) :
        magic1(VAL),
        _size(s), _elems(els), owning_elems(owns)
        , magic2(VAL) {
        std::cerr << "constructing array: " << this << " elems: " << _elems << ", length: " << _size << ", owning: " << owning_elems << ", magic1: " << magic1 << ", magic2: " << magic2 << std::endl;
        register_arr((Array<int> *) this);
    }
    
    Array(const Array<T>& a) :
        magic1(VAL),
        _size(a._size), _elems(a._elems), owning_elems(false)
        , magic2(VAL) {
        ASSERT_MSG(!a.owning_elems, "cannot copy array with ownership");
        register_arr((Array<int> *) this);
    }
    
    ~Array() {
        std::cerr << "destroying array: " << this << " elems: " << _elems << ", length: " << _size << ", owning: " << owning_elems << ", magic1: " << magic1 << ", magic2: " << magic2 << std::endl;
        ASSERT(magic1 == VAL && magic2 == VAL);
        if (owning_elems) delete[] _elems;
        unregister_arr((Array<int> *) this);
    }
    
    uint32 size() const { return _size; }
    
    T& operator[](uint32 i) { return _elems[i]; }
    
    const T& operator[](uint32 i) const { return _elems[i]; }
    
    T& get(uint32 i) { return this->operator[](i); }
    
    const T& get(uint32 i) const { return this->operator[](i); }

    void setDelete(bool do_delete) { owning_elems = do_delete; }

    Array<T> clone() const {
        T *elems = new T[_size];
        for (uint32 i = 0; i < _size; ++i)
            elems[i] = _elems[i];
        return Array<T>(elems, _size, true);
    }
};


#define DEFINE_ARRAY(name, type, ...)                                   \
    type CONCAT(_array_data_, name)[] = { __VA_ARGS__ };                \
    Array<type> name(CONCAT(_array_data_, name), ARRAY_LENGTH(CONCAT(_array_data_, name)))

#define DEFINE_CONST_ARRAY(name, type, ...)                             \
    type CONCAT(_array_data_, name)[] = { __VA_ARGS__ };                \
    const Array<type> name(CONCAT(_array_data_, name), ARRAY_LENGTH(CONCAT(_array_data_, name)))

#endif
