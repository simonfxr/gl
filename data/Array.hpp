#ifndef DATA_ARRAY_HPP
#define DATA_ARRAY_HPP

#include "defs.h"
#include "error/error.hpp"

template <typename T>
struct Array {
private:
    const uint32 _size;
    T * const _elems;
    bool del_elems;
public:
    Array(T *els, uint32 s, bool del = false) : _size(s), _elems(els), del_elems(del) {}
    Array(const Array<T>& a) : _size(a._size), _elems(a._elems), del_elems(false) {
        ASSERT_MSG(!a.del_elems, "copying array with ownership");
    }
    ~Array() { if (del_elems) delete[] _elems; }
    uint32 size() const { return _size; }
    void setDelete(bool del = true) { del_elems = del; }
    T& operator[](uint32 i) { return _elems[i]; }
    const T& operator[](uint32 i) const { return _elems[i]; }
};


#define DEFINE_ARRAY(name, type, ...)                                   \
    type CONCAT(_array_data_, name)[] = { __VA_ARGS__ };                \
    Array<type> name(CONCAT(_array_data_, name), ARRAY_LENGTH(CONCAT(_array_data_, name)))

#define DEFINE_CONST_ARRAY(name, type, ...)                             \
    type CONCAT(_array_data_, name)[] = { __VA_ARGS__ };                \
    const Array<type> name(CONCAT(_array_data_, name), ARRAY_LENGTH(CONCAT(_array_data_, name)))

#endif
