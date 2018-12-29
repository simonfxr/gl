#ifndef GE_TIME_HPP
#define GE_TIME_HPP

#include "sys/clock.hpp"
#include "sys/io/Stream.hpp"

#ifndef TIMING_DISABLE

#define measure_time(ret, op)                                                  \
    do {                                                                       \
        double _T0_ = ::sys::queryTimer();                                     \
        (op);                                                                  \
        ret = ::sys::queryTimer() - _T0_;                                      \
    } while (0)

#define time_op(...)                                                           \
    do {                                                                       \
        double _T0_ = ::sys::queryTimer();                                     \
        __VA_ARGS__;                                                           \
        double _diff_ = ::sys::queryTimer() - _T0_;                            \
        ::sys::io::stdout() << #__VA_ARGS__ << " took " << (_diff_ * 1000)     \
                            << " ms." << ::sys::io::endl;                      \
    } while (0)

#else

#define measure_time(ret, op)                                                  \
    do {                                                                       \
        (op);                                                                  \
        ret = 0;                                                               \
    } while (0)

#define time_op(op)                                                            \
    do {                                                                       \
        (op);                                                                  \
    } while (0)

#endif

#endif
