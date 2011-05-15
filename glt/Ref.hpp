#ifndef GLT_REF_HPP
#define GLT_REF_HPP

#include "defs.h"
#include "glt/error.hpp"
#include <iostream>

namespace glt {

namespace atomic {

struct Counter {
    uint32 x; // not thread safe
    Counter(uint32 initial) : x(initial) {}
    ~Counter() { ON_DEBUG(x = 0xDEADBEEF); }
    void inc() { DEBUG_ASSERT(x != 0xDEADBEEF); ++x; }
    uint32 decAndGet() { DEBUG_ASSERT(x != 0xDEADBEEF && x > 0); return --x; }
    uint32 get() { DEBUG_ASSERT(x != 0xDEADBEEF); return x; }
    bool alive() { bool ret = true; ON_DEBUG(ret = x != 0xDEADBEEF); ret = ret && x > 0; return ret; }
};

} // namespace atomic

namespace priv {

template <typename T, typename C, typename R>
struct RefBase {
protected:
    T *_ptr; // fat pointer, saves one indirection for pointer uses, but wastes space
    atomic::Counter *_cnt;

private:

    void retain() {
        ASSERT(C::get(_cnt) != 0);
        C::inc(_cnt);
    }
    
    void release() {
        if (C::decAndGet(_cnt) == 0) {
            delete _ptr;
            delete _cnt;
        }
    }

protected:

    RefBase(T *p, atomic::Counter *cnt) : _ptr(p), _cnt(cnt) { retain(); }

public:

    explicit RefBase(T *p = 0) : _ptr(p), _cnt(new atomic::Counter(1)) {}
    RefBase(const RefBase<T, C, R>& ref) : _ptr(ref._ptr), _cnt(ref._cnt)  { retain(); }

    ~RefBase() { release(); }

    RefBase<T, C, R>& operator =(const RefBase<T, C, R>& ref) {
        if (likely(_cnt != ref._cnt)) {
            release();
            _ptr = ref._ptr; _cnt = ref._cnt;
            retain();
        }
        return *this;
    }

    void set(T *p) {
        if (likely(p != _ptr)) {
            release(); // could reuse old counter, but makes debugging harder
            _ptr = p; _cnt = new atomic::Counter(1);
        }
    }

    uint32 refCount() const { return C::get(_cnt); }
    bool valid() const { return C::alive(_cnt); }
    
    const T& operator *() const { DEBUG_ASSERT(_ptr != 0); return *_ptr; }
    T& operator *() { DEBUG_ASSERT(_ptr != 0); return *_ptr; }

    const T *operator ->() const { DEBUG_ASSERT(_ptr != 0); return _ptr; }
    T *operator ->() { DEBUG_ASSERT(_ptr != 0); return _ptr; }

    const T *ptr() const { return _ptr; }
    T *ptr() { return _ptr; }

    bool operator ==(const R& ref) const { return _cnt == ref._cnt; }
    bool operator !=(const R& ref) const { return !(*this == ref); }
};

struct RefCnt {
    static void inc(atomic::Counter *cnt) { cnt->inc(); }
    static uint32 decAndGet(atomic::Counter *cnt) { return cnt->decAndGet(); }
    static uint32 get(atomic::Counter *cnt) { return cnt->get(); }
    static bool alive(atomic::Counter *cnt) { return cnt->alive(); }
};

struct WeakRefCnt : public RefCnt {
    static void inc(atomic::Counter *cnt) { UNUSED(cnt); }
    static uint32 decAndGet(atomic::Counter *cnt) { UNUSED(cnt); return 1; }
};

} // namespace priv

template <typename T> struct Ref;
template <typename T> struct WeakRef;

template <typename T>
struct Ref : public priv::RefBase<T, priv::RefCnt, Ref<T> > {
    explicit Ref(T *p = 0) : priv::RefBase<T, priv::RefCnt, Ref<T> >(p) {}
    Ref(const Ref<T>& ref) : priv::RefBase<T, priv::RefCnt, Ref<T> >(ref) {}
    Ref<T>& operator =(T *p) { this->set(p); return *this; }
    WeakRef<T> weak() const;
private:
    Ref(T *p, atomic::Counter *cnt) : priv::RefBase<T, priv::RefCnt, Ref<T> >(p, cnt) {}
    friend struct WeakRef<T>;
};

template <typename T>
struct WeakRef : public priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> > {
    explicit WeakRef(T *p = 0) : priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> >(p) {}
    WeakRef(const WeakRef<T>& ref) : priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> >(ref) {}
    WeakRef<T>& operator =(T *p) { this->set(p); return *this; }
    Ref<T> unweak() const;
private:
    WeakRef(T *p, atomic::Counter *cnt) : priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> >(p, cnt) {}
    friend struct Ref<T>;
};

template <typename T>
WeakRef<T> Ref<T>::weak() const { return WeakRef<T>(this->_ptr, this->_cnt); }

template <typename T>
Ref<T> WeakRef<T>::unweak() const {
    ASSERT(this->valid());
    return Ref<T>(this->_ptr, this->_cnt);
}

} // namespace glt

#endif
