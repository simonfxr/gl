#ifndef GLT_REF_HPP
#define GLT_REF_HPP

#include "defs.h"
#include "error/error.hpp"
#include <iostream>

namespace atomic {

static const int32 MARK = -0xAFAFAFAF;

struct Counter { // not thread safe
    int32 count; 
    Counter(int32 initial) : count(initial) {}
    ~Counter() { ON_DEBUG(count = MARK); }
    
    int32 getAndInc() { DEBUG_ASSERT(count != MARK); return count++; }
    int32 decAndGet() { DEBUG_ASSERT(count != MARK && count > 0); return --count; }
    void inc() { getAndInc(); }
    void dec() { decAndGet(); }
    int32 get() const { DEBUG_ASSERT(count != MARK); return count; }
    bool xchg(int32 expected, int32 *val) {
        if (count == expected) {
            count = *val;
            return true;
        } else {
            *val = count;
            return false;
        }
    }
};

struct RefCount {
    Counter strong, weak;
    RefCount(int32 s, int32 w) : strong(s), weak(w) {}
};

} // namespace atomic

namespace priv {

template <typename T, typename C, typename R>
struct RefBase {
protected:
    T *_ptr; // fat pointer, saves one indirection for pointer uses, but wastes space
    atomic::RefCount *_cnt;

protected:
    void retain() { C::retain(_cnt); }
    void release() { if (C::release(_cnt)) delete _ptr; }

    RefBase(T *p, atomic::RefCount *cnt) : _ptr(p), _cnt(cnt) { }
    
    void set(T *p) {
        if (likely(p != _ptr)) {
            release(); // could reuse old counter, but makes debugging harder
            _ptr = p;
            _cnt = new atomic::RefCount(1, 0);
        }
    }

public:
    explicit RefBase(T *p = 0) : _ptr(p), _cnt(new atomic::RefCount(1, 0)) {}
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

    uint32 refCount() const { return _cnt->strong.get(); }
    uint32 weakCount() const { return _cnt->weak.get(); }
    bool valid() const { return _cnt->strong.get() > 0; }
    
    const T *ptr() const { ON_DEBUG(_cnt->strong.get()); return _ptr; }
    T *ptr() { ON_DEBUG(_cnt->strong.get()); return _ptr; }

    bool same(const R& ref) const { return _cnt == ref._cnt; }

    bool operator ==(const T *p) const { return _ptr == p; }
    bool operator !=(const T *p) const { return _ptr != p; }
};

struct RefCnt {
    static void retain(atomic::RefCount *cnt) {
        DEBUG_ASSERT(cnt->strong.get() > 0);
        cnt->strong.inc();
    }
    
    static bool release(atomic::RefCount *cnt) {
        DEBUG_ASSERT(cnt->strong.get() > 0);
        if (cnt->strong.decAndGet() == 0) {
            int32 val = -1;
            if (cnt->weak.xchg(0, &val))
                delete cnt;
            return true;
        }
        return false;
    }
};

struct WeakRefCnt {
    static void retain(atomic::RefCount *cnt) {
        DEBUG_ASSERT(cnt->weak.get() > 0 || cnt->strong.get() > 0);
        cnt->weak.inc();
    }
    
    static bool release(atomic::RefCount *cnt) {
        DEBUG_ASSERT(cnt->weak.get() > 0);
        if (cnt->weak.decAndGet() == 0) {
            if (cnt->strong.get() == 0) {
                int32 val = -1;
                if (cnt->weak.xchg(0, &val))
                    delete cnt;
            }
        }
        return false;
    }
};

} // namespace priv

template <typename T> struct Ref;
template <typename T> struct WeakRef;

template <typename T>
struct Ref : public priv::RefBase<T, priv::RefCnt, Ref<T> > {
    explicit Ref(T *p = 0) : priv::RefBase<T, priv::RefCnt, Ref<T> >(p) {}
    Ref(const Ref<T>& ref) : priv::RefBase<T, priv::RefCnt, Ref<T> >(ref) {}
    Ref<T>& operator =(T *p) { this->set(p); return *this; }
    WeakRef<T> weak();
    operator bool() const { return this->_ptr != 0; }
    const T& operator *() const { DEBUG_ASSERT(this->valid() && this->_ptr != 0); return *this->_ptr; }
    T& operator *() { DEBUG_ASSERT(this->valid() && this->_ptr != 0); return *this->_ptr; }

    const T *operator ->() const { DEBUG_ASSERT(this->valid() && this->_ptr != 0); return this->_ptr; }
    T *operator ->() { DEBUG_ASSERT(this->valid() && this->_ptr != 0); return this->_ptr; }
private:
    Ref(T *p, atomic::Counter *cnt) : priv::RefBase<T, priv::RefCnt, Ref<T> >(p, cnt) { this->retain(); }
    friend struct WeakRef<T>;
};

template <typename T>
struct WeakRef : public priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> > {
    WeakRef() : priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> >(0, new atomic::RefCount(0, 1)) {}
    WeakRef(const WeakRef<T>& ref) : priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> >(ref) {}
    bool unweak(Ref<T> *dest);
private:
    WeakRef(T *p, atomic::RefCount *cnt) : priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> >(p, cnt) { this->retain(); }
    friend struct Ref<T>;
};

template <typename T>
WeakRef<T> Ref<T>::weak() { return WeakRef<T>(this->_ptr, this->_cnt); }

template <typename T>
bool WeakRef<T>::unweak(Ref<T> *dest) {
    if (this->_cnt->strong.getAndInc() > 0) {
        dest->release();
        dest->_ptr = this->_ptr;
        dest->_cnt = this->_cnt;
        return true;
    } else {
        this->_cnt->strong.dec();
        return false;
    }
}

template <typename T>
Ref<T> makeRef(T *p) { return Ref<T>(p); }

#endif
