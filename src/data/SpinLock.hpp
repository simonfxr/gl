#ifndef DATA_SPINLOCK_HPP
#define DATA_SPINLOCK_HPP

#include <sched.h>

// realy dumb (just for fun)
struct SpinLock {
    int32 lock;

    SpinLock() : lock(0) {}

    void acquire() {
        int iter = 0;
        while (__sync_lock_test_and_set(&lock, 1) != 0) {
            if (iter++ > 5) {
                sched_yield();
                iter = 0;
            }
        }
    }

    void release() {
//        __sync_synchronize();
        __sync_lock_release(&lock);
    }
};

#endif