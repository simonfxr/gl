#ifndef DATA_SHARED_ARRAY_HPP
#define DATA_SHARED_ARRAY_HPP

#include "data/Array.hpp"

template <typename T>
struct SharedArray : public Array<T> {
public:

    SharedArray() : Array<T>() {}

    SharedArray(defs::size s, T *elems) :
        Array<T>(s, elems) {}

    ~SharedArray() {
        this->_elems = 0; // prevent deletion
    }

    void set(defs::size s, T *elems) {
        this->_size = s;
        this->_elems = elems;
    }
};

#define DEFINE_ARRAY(name, type, ...)                                   \
    static type CONCAT(_array_data_, name)[] = { __VA_ARGS__ };                \
    SharedArray<type> name(ARRAY_LENGTH(CONCAT(_array_data_, name)), CONCAT(_array_data_, name))

#define DEFINE_CONST_ARRAY(name, type, ...)                             \
    static type CONCAT(_array_data_, name)[] = { __VA_ARGS__ };         \
    const SharedArray<type> name(ARRAY_LENGTH(CONCAT(_array_data_, name)), CONCAT(_array_data_, name))

// FIXME: can this be made portable (maybe lambda's) ?
// #ifdef GNU_EXTENSIONS 
#define CONST_ARRAY(type, ...) ({                                       \
            static type CONCAT(_array_data_, _anon_arr)[] = { __VA_ARGS__ }; \
            static const SharedArray<type> _anon_arr(                   \
                ARRAY_LENGTH(CONCAT(_array_data_, _anon_arr)),          \
                CONCAT(_array_data_, _anon_arr));                       \
            _anon_arr;                                                  \
        })
// #endif

#endif
