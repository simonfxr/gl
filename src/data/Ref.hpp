#ifndef DATA_REF_HPP
#define DATA_REF_HPP

#include "defs.hpp"
#include "err/err.hpp"
#include <iostream>

#ifdef REF_CONCURRENT
#include "data/Atomic.hpp"
#endif

namespace atomic {

using namespace defs;

static const int32 MARK = int32(0xAFAFAFAF);

static const int32 DEAD_STRONG = -0x0FFFFFFF;

#ifdef REF_CONCURRENT

struct Counter {
    Atomic<int32> count;

    Counter(int32 initial) :
        count(initial) {}

    ~Counter() { ON_DEBUG(count.set(MARK)); }

    int32 getAndInc() { DEBUG_ASSERT(count.get() != MARK); return count.getAndInc(); }
    int32 decAndGet() { DEBUG_ASSERT(count.get() != MARK); return count.decAndGet(); }
    void inc() { getAndInc(); }
    void dec() { decAndGet(); }
    int32 get() const { int32 val = count.get(); DEBUG_ASSERT(val != MARK); return val; }
    bool xchg(int32 expected, int32 *val) {
        return count.xchg(expected, val);
    }    
};

#else

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
        DEBUG_ASSERT(count != MARK);
        if (count == expected) {
            count = *val;
            return true;
        } else {
            *val = count;
            return false;
        }
    }
};

#endif

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
    mutable atomic::RefCount *_cnt;

protected:
    void retain() const { C::retain(_cnt); }
    void release() const { if (C::release(_cnt)) delete _ptr; }

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
        ref.retain();
        release();
        _ptr = ref._ptr; _cnt = ref._cnt;
        return *this;
    }

    defs::uint32 refCount() const { return _cnt->strong.get(); }
    defs::uint32 weakCount() const { return _cnt->weak.get(); }
    bool valid() const { return _cnt->strong.get() > 0; }
    const void *identity() const { return _cnt; }
    
    const T *ptr() const { ON_DEBUG(_cnt->strong.get()); return _ptr; }
    T *ptr() { ON_DEBUG(_cnt->strong.get()); return _ptr; }

    template <typename T2, typename C2, typename R2>
    bool same(const RefBase<T2, C2, R2>& ref) const { return identity() == ref.identity(); }

    bool operator ==(const T *p) const { return _ptr == p; }
    bool operator !=(const T *p) const { return _ptr != p; }

    operator bool() const { return this->_ptr != 0; }
};

template <typename T, typename C, typename R>
inline bool operator ==(const T * lhs, const RefBase<T, C, R>& rhs) {
    return rhs == lhs;
}

template <typename T, typename C, typename R>
inline bool operator !=(const T * lhs, const RefBase<T, C, R>& rhs) {
    return rhs != lhs;
}

struct RefCnt {
    static void retain(atomic::RefCount *cnt) {
        DEBUG_ASSERT(cnt->strong.get() > 0);
        cnt->strong.inc();
    }

    
    
    static bool release(atomic::RefCount *cnt) {
        defs::int32 alive = cnt->strong.decAndGet();
        DEBUG_ASSERT(alive >= 0);
        
        if (alive == 0) {
            // we are the last strong, only weak are remaining
            
            defs::int32 val = -1;
            if (cnt->weak.xchg(0, &val)) {
                // last one, delete counter
                delete cnt;
                return true;
            }

            // highly negative value,
            // makes conversion from weak to strong easier
            defs::int32 dead_val = atomic::DEAD_STRONG;; 

            if (cnt->strong.xchg(0, &dead_val)) {
                // no weak tried to convert
                return true;
            }
        }

        return false;
    }

    // calling this function implies at least one weak count is remaining
    static bool tryRetain(atomic::RefCount *cnt) {
        defs::int32 value = cnt->strong.getAndInc();
        if (value >= 0) {
            // the case of value == 0 is slightly subtle
            // as at least one weak is alive at this point
            // the xchg on weak in RefCnt::release will fail
            // the xchg on strong in RefCnt::release will also fail
            // as we bumped the strong count above zero
            return true;
        } else {
            cnt->strong.dec();
            return false;
        }
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
            if (cnt->strong.get() < 0) {
                defs::int32 val = -1;
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
template <typename T> struct RefValue;

template <typename T>
struct Ref : public priv::RefBase<T, priv::RefCnt, Ref<T> > {
    explicit Ref(T *p = 0) : priv::RefBase<T, priv::RefCnt, Ref<T> >(p) {}
    
    explicit Ref(RefValue<T>&);
    
    Ref(const Ref<T>& ref) : priv::RefBase<T, priv::RefCnt, Ref<T> >(ref) {}
    
    Ref<T>& operator =(T *p) { this->set(p); return *this; }
    
    WeakRef<T> weak();
    
    const T& operator *() const { DEBUG_ASSERT(this->valid() && this->_ptr != 0); return *this->_ptr; }
    
    T& operator *() { DEBUG_ASSERT(this->valid() && this->_ptr != 0); return *this->_ptr; }

    const T *operator ->() const { DEBUG_ASSERT(this->valid() && this->_ptr != 0); return this->_ptr; }
    
    T *operator ->() { DEBUG_ASSERT(this->valid() && this->_ptr != 0); return this->_ptr; }
    
    template <typename U>
    Ref<U>& cast() { return reinterpret_cast<Ref<U>&>(*this); }
    
    template <typename U>
    const Ref<U>& cast() const { return reinterpret_cast<const Ref<U>&>(*this); }
    
private:
    friend struct WeakRef<T>;
};

template <typename T>
struct WeakRef : public priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> > {
    
    WeakRef() : priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> >(0, new atomic::RefCount(atomic::DEAD_STRONG, 1)) {}
    
    WeakRef(const WeakRef<T>& ref) : priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> >(ref) {}
    
    bool unweak(RefValue<T> *dest);
    
    bool unweak(Ref<T> *dest);

    template <typename U>
    WeakRef<U>& cast() { return static_cast<WeakRef<U>&>(*this); }
    
    template <typename U>
    const WeakRef<U>& cast() const { return static_cast<const WeakRef<U>&>(*this); }
    
private:
    
    WeakRef(T *p, atomic::RefCount *cnt) : priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> >(p, cnt) { this->retain(); }
    
    friend struct Ref<T>;
};

template <typename T>
WeakRef<T> Ref<T>::weak() { return WeakRef<T>(this->_ptr, this->_cnt); }

template <typename T>
struct RefValue {
private:
    T *_ptr;
    atomic::RefCount *_cnt;

    void unset() {
        if (_cnt != 0) {
            Ref<T> ref(*this); // force release
        }
        
        _ptr = 0;
        _cnt = 0;
    }
    
public:

    RefValue() : _ptr(0), _cnt(0) {}

    ~RefValue() { unset(); }

private:
    
    RefValue(T *ptr, atomic::RefCount *cnt) :
        _ptr(ptr), _cnt(cnt) {}

    friend struct Ref<T>;
    friend struct WeakRef<T>;
};

template <typename T>
Ref<T>::Ref(RefValue<T>& rv) :
    priv::RefBase<T, priv::RefCnt, Ref<T> >(rv._ptr, rv._cnt)
{
    ASSERT(rv._cnt != 0);
    rv._ptr = 0;
    rv._cnt = 0;
}

template <typename T>
bool WeakRef<T>::unweak(RefValue<T> *dest) {
    if (dest->_cnt == this->_cnt)
        return true;

    if (priv::RefCnt::tryRetain(this->_cnt)) {
        dest->unset();
        dest->_ptr = this->_ptr;
        dest->_cnt = this->_cnt;
        return true;
    } else {
        return false;
    }
}

template <typename T>
bool WeakRef<T>::unweak(Ref<T> *dest) {
    if (dest->_cnt == this->_cnt)
        return true;
    
    if (priv::RefCnt::tryRetain(this->_cnt)) {
        dest->release();
        dest->_ptr = this->_ptr;
        dest->_cnt = this->_cnt;
        return true;
    } else {
        return false;
    }
}

template <typename T>
Ref<T> makeRef(T *p) { return Ref<T>(p); }

#endif
