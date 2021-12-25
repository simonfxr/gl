#ifndef GE_TIME_HPP
#define GE_TIME_HPP

#include "sys/clock.hpp"
#include "sys/io/Stream.hpp"

#define measure_time_name(name, ret, op)                                       \
    do {                                                                       \
        auto name = ::sys::queryTimer();                                       \
        (op);                                                                  \
        ret = ::sys::queryTimer() - name;                                      \
    } while (0)

#define measure_time(ret, op) measure_time_name(HU_COUNTER, ret, op)

#define time_op_name(name, ...)                                                \
    do {                                                                       \
        double name = ::sys::queryTimer();                                     \
        __VA_ARGS__;                                                           \
        double _diff_ = ::sys::queryTimer() - name;                            \
        ::sys::io::stdout()                                                    \
          << #__VA_ARGS__ << " took " << (_diff_ * 1000) << " ms.\n";          \
    } while (0)

#define time_op(...) time_op_name(HU_COUNTER, __VA_ARGS__)

#endif
