#ifndef DATA_ATOMIC_HPP
#define DATA_ATOMIC_HPP

template <typename T>
struct Atomic {
    T value;

    Atomic(T initial) :
        value(initial) {}

    Atomic() {}

    T getLazy() const { return value; }

    T get() const { return *(const volatile T *) &value; }
    
    void set(T v) { *(volatile T *) &value = v; }

    void lazySet(T v) { value = v; }

    T getAndAdd(T v) {
        return __sync_fetch_and_add(&value, v);
    }

    T getAndInc() {
        return getAndAdd(1);
    }

    T addAndGet(T v) {
        return __sync_add_and_fetch(&value, v);
    }

    T incAndGet() {
        return addAndGet(1);
    }    

    T getAndSub(T v) {
        return __sync_fetch_and_sub(&value, v);
    }

    T getAndDec() {
        return getAndSub(1);
    }

    T subAndGet(T v) {
        return __sync_sub_and_fetch(&value, v);
    }

    T decAndGet() {
        return subAndGet(1);
    }

    void inc() {
        getAndInc();
    }

    void dec() {
        getAndDec();
    }

    bool xchg(T expected, T *new_val) {
        T current = __sync_val_compare_and_swap(&value, expected, *new_val);
        *new_val = current;
        return current == expected;
    }
};

#endif
