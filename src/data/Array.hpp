#ifndef DATA_ARRAY_HPP
#define DATA_ARRAY_HPP

#include "defs.hpp"
#include "err/err.hpp"

#include <string>
#include <sstream>

#define ARRAY_MARK 0xDEADBEEF
#define CHECK_ARRAY() DEBUG_ASSERT(mark == ARRAY_MARK)

template <typename T>
struct Array {
private:

    DEBUG_DECL(defs::uint32 mark;)
    const defs::size _size;
    T * const _elems;
    bool owning_elems;

public:
    // disambiguate constructor
    enum Ownership {
        Shared,
        Owned
    };
    
    Array(defs::size s, Ownership own = Shared) :
        _size(ASSERT_SIZE(s)), _elems(new T[UNSIZE(s)]), owning_elems(own == Owned) {
        ON_DEBUG(mark = ARRAY_MARK);
    }
    
    Array(T *els, defs::size s, Ownership own = Shared) :
        _size(ASSERT_SIZE(s)), _elems(els), owning_elems(own == Owned) {
        ON_DEBUG(mark = ARRAY_MARK);
    }
    
    Array(const Array<T>& a) :
        _size(a._size), _elems(a._elems), owning_elems(false) {
        ON_DEBUG(mark = a.mark);
        CHECK_ARRAY();
        ASSERT_MSG(!a.owning_elems, "cannot copy array with ownership");
    }
    
    ~Array() {
        DEBUG_DECL(mark = 0xDEAD;)
        if (owning_elems) delete[] _elems;
    }
    
    defs::size size() const { CHECK_ARRAY(); return _size; }
    
    T& operator[](defs::index i) { CHECK_ARRAY(); return _elems[i]; }
    
    const T& operator[](defs::index i) const { CHECK_ARRAY(); return _elems[i]; }
    
    T& at(defs::index i) { CHECK_ARRAY(); return this->operator[](i); }
    
    const T& at(defs::index i) const { CHECK_ARRAY(); return this->operator[](i); }

    void setDelete(bool do_delete = true) { CHECK_ARRAY(); owning_elems = do_delete; }

    Array<T> clone() const {
        CHECK_ARRAY(); 
        T *elems = new T[UNSIZE(_size)];
        for (defs::index i = 0; i < _size; ++i)
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
