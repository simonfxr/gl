#ifndef DATA_REF_HPP
#define DATA_REF_HPP

#include "defs.hpp"
#include "err/err.hpp"
#include "data/Atomic.hpp"

#ifndef REF_COUNTER_TYPE
#  ifdef REF_CONCURRENT
#    define REF_COUNTER_TYPE ::atomic::AtomicCounter
#  else
#    define REF_COUNTER_TYPE ::atomic::SeqCounter
#  endif
#endif

namespace atomic {

using namespace defs;

static const int32 MARK = int32(0xAFAFAFAF);

struct AtomicCounter {
    Atomic<int32> count;

    AtomicCounter(int32 initial) :
        count(initial) {}

    ~AtomicCounter() { ON_DEBUG(count.set(MARK)); }

    int32 decAndGet() { DEBUG_ASSERT(count.get() != MARK); return count.decAndGet(); }
    void inc() { DEBUG_ASSERT(count.get() != MARK); count.inc(); }
    void dec() { decAndGet(); }
    int32 get() const { int32 val = count.get(); DEBUG_ASSERT(val != MARK); return val; }
    bool xchg(int32 expected, int32 *val) {
        return count.xchg(expected, val);
    }    
};

struct SeqCounter { // not thread safe
    int32 count; 
    SeqCounter(int32 initial) : count(initial) {}
    ~SeqCounter() { ON_DEBUG(count = MARK); }
    
    int32 decAndGet() { DEBUG_ASSERT(count != MARK && count > 0); return --count; }
    void inc() { DEBUG_ASSERT(count != MARK && count > 0); count++; }
    void dec() { decAndGet(); }
    int32 get() const { DEBUG_ASSERT(count != MARK && count >= 0); return count; }
    bool xchg(int32 expected, int32 *val) {
        DEBUG_ASSERT(count != MARK && expected >= 0 && *val >= 0);
        if (count == expected) {
            count = *val;
            return true;
        } else {
            *val = count;
            return false;
        }
    }
};

template <typename Counter>
struct RefCount {
    Counter strong; // number of non weak refs
                    // invariant: 0 is a terminal state,
                    // if strong gets decremented to 0 it stays 0,
                    // this makes sure that weak refs can easily be
                    // converted to non weak refs
    
    Counter weak;   // if strong == 0 then number of weak refs
                    // if strong >  0 then number of weak refs + 1
    RefCount(int32 s, int32 w) : strong(s), weak(w) {}
};

} // namespace atomic

namespace priv {

template <typename T, typename C, typename R>
struct RefBase {
    
protected:
    T *_ptr; // fat pointer, saves one indirection for pointer uses, but wastes space
    typename C::ref_count_type *_cnt;

protected:
    void retain() const { if (_cnt != 0) C::retain(_cnt); }
    void release() const { if (_cnt != 0 && C::release(_cnt)) delete _ptr; }

    template <typename U, typename V, typename W>
    RefBase(const RefBase<U, V, W>& ref) : _ptr(ref._ptr), _cnt(ref._cnt) { retain(); }

    template <typename O>
    void set(O *p) {
        if (likely(p != _ptr)) {
            release(); // could reuse old counter, but makes debugging harder
            _ptr = p;
            _cnt = (p != 0 ? C::newRefCount() : 0);
        }
    }

public:
    template <typename O>
    explicit RefBase(O *p = 0) : _ptr(p), _cnt(p != 0 ? C::newRefCount() : 0) {}
    
    RefBase(const RefBase<T, C, R>& ref) : _ptr(ref._ptr), _cnt(ref._cnt)  { retain(); }
    
    template <typename O, typename S>
    RefBase(const RefBase<O, C, S>& ref) : _ptr(ref._ptr), _cnt(ref._cnt) { retain(); }
    
    ~RefBase() { release(); }

    RefBase<T, C, R>& operator =(const RefBase<T, C, R>& ref) {
        ref.retain();
        release();
        _ptr = ref._ptr; _cnt = ref._cnt;
        return *this;
    }

    template <typename O, typename S>
    RefBase<T, C, R>& operator =(const RefBase<O, C, S>& ref) {
        ref.retain();
        release();
        _ptr = ref._ptr; _cnt = ref._cnt;
        return *this;
    }

    bool valid() const { return _cnt ? _cnt->strong.get() > 0 : true; }
    const void *identity() const { return _cnt; }
    
    const T *ptr() const { ON_DEBUG(_cnt && _cnt->strong.get()); return _ptr; }
    T *ptr() { ON_DEBUG(_cnt && _cnt->strong.get()); return _ptr; }

    template <typename T2, typename C2, typename R2>
    bool same(const RefBase<T2, C2, R2>& ref) const { return identity() == ref.identity(); }

    template <typename O>
    bool operator ==(const O *p) const { return _ptr == p; }
    
    template <typename O>
    bool operator !=(const O *p) const { return _ptr != p; }

    operator bool() const { return this->_ptr != 0; }

    template <typename X, typename Y, typename Z>
    friend struct RefBase;
};

template <typename T, typename O, typename C, typename R>
inline bool operator ==(const O* lhs, const RefBase<T, C, R>& rhs) {
    return rhs == lhs;
}

template <typename T, typename O, typename C, typename R>
inline bool operator !=(const O* lhs, const RefBase<T, C, R>& rhs) {
    return rhs != lhs;
}

struct RefCnt {

    typedef REF_COUNTER_TYPE counter_type;
    typedef atomic::RefCount<counter_type> ref_count_type;

    static void retain(ref_count_type *cnt) {
        DEBUG_ASSERT(cnt->strong.get() > 0);
        cnt->strong.inc();
    }

    static bool release(ref_count_type *cnt) {
        defs::int32 alive = cnt->strong.decAndGet();
        DEBUG_ASSERT(alive >= 0);
        
        if (alive == 0) {
            // we are the last strong, only weak are remaining
            defs::int32 val = 0;
            if (cnt->weak.xchg(1, &val))
                delete cnt;
            return true;
        }

        return false;
    }

    static bool strongFromWeak(ref_count_type *cnt) {
        DEBUG_ASSERT(cnt->weak.get() > 0);
        for (;;) {
            defs::int32 alive = cnt->strong.get();
            if (alive == 0)
                return false;
            defs::int32 nalive = alive + 1;
            if (cnt->strong.xchg(alive, &nalive))
                return true;
        }
    }

    static ref_count_type *newRefCount() {
        return new ref_count_type(1, 1);
    }
};

struct WeakRefCnt {

    typedef REF_COUNTER_TYPE counter_type;
    typedef atomic::RefCount<counter_type> ref_count_type;
    
    static void retain(ref_count_type *cnt) {
        DEBUG_ASSERT(cnt->weak.get() > 0);
        cnt->weak.inc();
    }
    
    static bool release(ref_count_type *cnt) {
        DEBUG_ASSERT(cnt->weak.get() > 0);
        if (cnt->weak.decAndGet() == 0) {
            // weak == 0 implies strong == 0
            DEBUG_ASSERT(cnt->strong.get() == 0);
            return true;
        }
        return false;
    }

    static ref_count_type *newRefCount() {
        return new ref_count_type(0, 1);
    }
};

} // namespace priv

template <typename T> struct Ref;
template <typename T> struct WeakRef;

template <typename T>
struct Ref : public priv::RefBase<T, priv::RefCnt, Ref<T> > {

    typedef T ptr_type;
    typedef Ref<T> type;
    typedef priv::RefBase<T, priv::RefCnt, Ref<T> > base_type;

    Ref() : base_type(static_cast<T *>(0)) {}

    template <typename O>
    explicit Ref(O *p) : base_type(p) {}

    template <typename O>
    explicit Ref(WeakRef<O>& ref) :
        base_type(static_cast<T *>(0))
    {
        ref.strong(this);
    }

    Ref(const Ref<T>& ref) : base_type(ref) {}

    template <typename O>
    Ref(const Ref<O>& ref) : base_type(ref) {}
    
    Ref<T>& operator =(T *p) { this->set(p); return *this; }

    template <typename O>
    Ref<T>& operator =(O *p) { this->set(p); return *this; }

    WeakRef<T> weak() { return WeakRef<T>(*this); }
    
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

    typedef T ptr_type;
    typedef WeakRef<T> type;
    typedef priv::RefBase<T, priv::WeakRefCnt, WeakRef<T> > base_type;
    
    WeakRef() : base_type(static_cast<T *>(0)) {}
    
    WeakRef(const WeakRef<T>& ref) : base_type(ref) {}

    template <typename O>
    WeakRef(const WeakRef<O>& ref) : base_type(ref) {}

    template <typename O>
    explicit WeakRef(Ref<O>& ref) : base_type(ref) {}

    template <typename O>
    bool strong(Ref<O> *dest) {
        if (dest->_cnt == this->_cnt)
            return true;
        
        if (priv::RefCnt::strongFromWeak(this->_cnt)) {
            dest->release();
            dest->_ptr = this->_ptr;
            dest->_cnt = this->_cnt;
            return true;
        } else {
            return false;
        }
    }

    template <typename O>
    Ref<O> strong() {
        Ref<O> ref;
        strong(&ref);
        return ref;
    }

    template <typename U>
    WeakRef<U>& cast() { return reinterpret_cast<WeakRef<U>&>(*this); }
    
    template <typename U>
    const WeakRef<U>& cast() const { return reinterpret_cast<const WeakRef<U>&>(*this); }
    
private:
    friend struct Ref<T>;
};

template <typename T>
inline Ref<T> makeRef(T *p) { return Ref<T>(p); }

#undef REF_COUNTER_TYPE

#endif
