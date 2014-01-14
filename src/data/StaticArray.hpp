#ifndef DATA_STATIC_ARRAY_HPP
#define DATA_STATIC_ARRAY_HPP

#include "data/Array.hpp"

#define DEFINE_CONST_ARRAY(name, type, ...)                             \
    static type CONCAT(_array_data_, name)[] = { __VA_ARGS__ };         \
    const Array<type> name = { ARRAY_LENGTH(CONCAT(_array_data_, name)), CONCAT(_array_data_, name) }

// FIXME: can this be made portable (maybe lambda's) ?
// #ifdef GNU_EXTENSIONS 
#define CONST_ARRAY(type, ...) ({                                       \
            static type CONCAT(_array_data_, _anon_arr)[] = { __VA_ARGS__ }; \
            static const Array<type> _anon_arr = {                      \
                ARRAY_LENGTH(CONCAT(_array_data_, _anon_arr)),          \
                CONCAT(_array_data_, _anon_arr) };                      \
            _anon_arr;                                                  \
        })
// #endif


#define DEFINE_ARRAY(name, type, ...)                                   \
    static type CONCAT(_array_data_, name)[] = { __VA_ARGS__ };                \
    SharedArray<type> name(ARRAY_LENGTH(CONCAT(_array_data_, name)), CONCAT(_array_data_, name))

#endif

