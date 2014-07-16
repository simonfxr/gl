#ifndef DATA_REF_NEW_HPP
#define DATA_REF_NEW_HPP

#include <cstddef>
#include <type_traits>

#ifdef GNU_EXTENSIONS
#  include "data/Atomic.hpp"
#endif
#include "err/err.hpp"

namespace _priv {

using namespace defs;

DEBUG_DECL(static const int32 MARK = int32(0xAFAFAFAF);)

#ifdef GNU_EXTENSIONS
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
#endif

struct SeqCounter {
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

template <typename Counter>
struct StrongCount {
    typedef RefCount<Counter> ref_count;

    static void retain(ref_count *cnt) {
        DEBUG_ASSERT(cnt->strong.get() > 0);
        cnt->strong.inc();
    }

    static bool release(ref_count *cnt) {
        defs::int32 alive = cnt->strong.decAndGet();
        DEBUG_ASSERT(alive >= 0);

        if (alive == 0) {
            // we are the last strong, only weak are remaining
            if (cnt->weak.decAndGet() == 0)
                delete cnt;
            return true;
        }

        return false;
    }

    static ref_count *make() {
        return new ref_count(1, 1);
    }
};

template <typename Counter>
struct WeakCount {
    typedef RefCount<Counter> ref_count;

    static void retain(ref_count *cnt) {
        DEBUG_ASSERT(cnt->weak.get() > 0);
        cnt->weak.inc();
    }

    static void release(ref_count *cnt) {
        DEBUG_ASSERT(cnt->weak.get() > 0);
        if (cnt->weak.decAndGet() == 0) {
            // weak == 0 implies strong == 0
            DEBUG_ASSERT(cnt->strong.get() == 0);
            delete cnt;
        }
    }
    
    static bool strongFromWeak(ref_count *cnt) {
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
};

} // namespace _priv

template <typename T, typename C>
struct Ref;

template <typename T, typename C>
struct WeakRef;

template <typename T, typename C = _priv::SeqCounter>
struct Ref {
private:

    typedef _priv::StrongCount<C> counter;
    typedef typename counter::ref_count ref_count;

    ref_count *_ref_cnt;
    T * _ptr;

    template <typename O>
    Ref(int /* for overloading */, O *ptr) :
        _ref_cnt(ptr ? counter::make() : nullptr), _ptr(ptr)
    {}

    void retain() const {
        if (_ref_cnt)
            counter::retain(_ref_cnt);
    }
    
    void release() const {
        if (_ref_cnt && counter::release(_ref_cnt))
            delete _ptr;
    }

    template <typename O, typename D>
    friend struct WeakRef;
    
    template <typename O, typename D>
    friend struct Ref;

public:

    Ref() : Ref(0, static_cast<T *>(nullptr)) {}

    Ref(std::nullptr_t) : Ref(0, nullptr) {}

    Ref(T *p) : Ref(0, p) {}

    template <typename O>
    Ref(O *p) : Ref(0, p) {}

    Ref(const Ref<T, C>& ref) :
        _ref_cnt(ref._ref_cnt), _ptr(ref._ptr)
    { retain(); }

    template <typename O>
    Ref(const Ref<O, C>& ref) :
        _ref_cnt(ref._ref_cnt), _ptr(ref._ptr)
    { retain(); }

    template <typename O>
    Ref(WeakRef<O, C>& ref) : Ref() {
        //   *this = ref.strong();
        ref.strong(this);
    }

    ~Ref() { release(); }

    T *ptr() { return _ptr; }
    const T *ptr() const { return _ptr; }

    const T& operator *() const {
        DEBUG_ASSERT(_ptr); return *_ptr;
    }

    T& operator *() {
        DEBUG_ASSERT(_ptr); return *_ptr;
    }

    const T *operator ->() const {
        DEBUG_ASSERT(_ptr); return _ptr; }
    
    T *operator ->() {
        DEBUG_ASSERT(_ptr); return _ptr;
    }

    operator bool() const {
        return _ptr != nullptr;
    }

    template <typename U>
    bool operator ==(const Ref<U, C>& rhs) const {
        return _ref_cnt == rhs._ref_cnt;
    }

    template <typename U>
    bool operator ==(const WeakRef<U, C>& rhs) const {
        return _ref_cnt == rhs._ref_cnt;
    }

    Ref<T, C>& operator =(const Ref<T, C>& ref) {
        ref.retain();
        release();
        _ref_cnt = ref._ref_cnt; _ptr = ref._ptr;
        return *this;
    }

    template <typename O>
    Ref<T, C>& operator =(Ref<O, C>& ref) {
        ref.retain();
        release();
        _ref_cnt = ref._ref_cnt; _ptr = ref._ptr;
        return *this;
    }

    template <typename O>
    Ref<T, C>& operator =(O *ptr) {
        release();
        _ref_cnt = (ptr ? counter::make() : nullptr);
        _ptr = ptr;
        return *this;
    }

    WeakRef<T, C> weak() {
        return WeakRef<T, C>(*this);
    }

    template <typename U>
    typename std::enable_if<std::is_base_of<T, U>::value,
                            Ref<U, C>&>::type
    cast() { return reinterpret_cast<Ref<U, C>&>(*this); }

    template <typename U>
    typename std::enable_if<std::is_base_of<T, U>::value,
                            const Ref<U, C>& >::type
    cast() const { return reinterpret_cast<const Ref<U, C>&>(*this); }
};

template <typename T, typename C = _priv::SeqCounter>
struct WeakRef {
private:

    typedef _priv::WeakCount<C> counter;
    typedef typename counter::ref_count ref_count;

    ref_count *_ref_cnt;
    T *_ptr;

    template <typename O>
    WeakRef(ref_count *ref_cnt, O *ptr)
        : _ref_cnt(ref_cnt), _ptr(ptr) {}

    void retain() const {
        if (_ref_cnt)
            counter::retain(_ref_cnt);
    }

    void release() const {
        if (_ref_cnt)
            counter::release(_ref_cnt);
    }

    template <typename O, typename D>
    friend struct Ref;

    template <typename O>
    bool strong(Ref<O, C> *dest) {

        if (!_ref_cnt) {
            DEBUG_ASSERT(_ptr == nullptr);
            return false;
        }

        if (dest->_ref_cnt == _ref_cnt)
            return true;

        DEBUG_ASSERT(_ref_cnt != nullptr);
        if (counter::strongFromWeak(_ref_cnt)) {
            dest->release();
            dest->_ptr = _ptr; dest->_ref_cnt = _ref_cnt;
            return true;
        }

        return false;
    }

public:

    WeakRef() : WeakRef<T>(nullptr, static_cast<T *>(nullptr)) {}

    template <typename O>
    WeakRef(Ref<O, C>& ref) : WeakRef(ref._ref_cnt, ref._ptr) {
        retain();
    }

    WeakRef(const WeakRef<T, C>& ref) : WeakRef(ref._ref_cnt, ref._ptr) {
        retain();
    }

    template <typename O>
    WeakRef(const WeakRef<O, C>& ref) : WeakRef(ref._ref_cnt, ref._ptr) {
        retain();
    }

    ~WeakRef() { release(); }

    WeakRef<T, C>& operator =(const WeakRef<T, C>& ref) {
        ref.retain();
        release();
        _ref_cnt = ref._ref_cnt; _ptr = ref._ptr;
        return *this;
    }

    template <typename O>
    WeakRef<T, C>& operator =(WeakRef<O, C>& ref) {
        ref.retain();
        release();
        _ref_cnt = ref._ref_cnt; _ptr = ref._ptr;
        return *this;
    }

    Ref<T, C> strong() {
        Ref<T, C> ref;
        strong(&ref);
        return ref;
    }

    template <typename U>
    typename std::enable_if<std::is_base_of<T, U>::value,
                            WeakRef<U, C>&>::type
    cast() {
        return reinterpret_cast<WeakRef<U, C>&>(*this);
    }
    
    template <typename U>
    typename std::enable_if<std::is_base_of<T, U>::value,
                            const WeakRef<U, C>&>::type
    cast() const {
        return reinterpret_cast<const WeakRef<U, C>&>(*this); 
    }

    template <typename O>
    bool operator ==(const Ref<O, C>& rhs) const {
        return rhs == *this;
    }

    template <typename O>
    bool operator ==(const WeakRef<O, C>& rhs) const {
        return _ref_cnt == rhs._ref_cnt;
    }

    template <typename O>
    bool operator ==(const O *ptr) const {
        return _ptr == ptr;
    }
};

template <typename T, typename C = _priv::SeqCounter>
Ref<T> makeRef(T *p) { return Ref<T, C>(p); }

#endif
