#ifndef GE_TIME_HPP
#define GE_TIME_HPP

#include "sys/clock.hpp"

#ifndef TIMING_DISABLE

#define measure_time(ret, op) do {                                      \
        float _T0_ = ::sys::queryTimer();                               \
        (op);                                                           \
        ret = ::sys::queryTimer() - _T0_;                               \
    } while (0)

#define time(op) do {                                                   \
        float _T0_ = ::sys::queryTimer();                               \
        (op);                                                           \
        float _diff_ = ::sys::queryTimer() - _T0_;                      \
        ::std::cerr << #op << " took " << (_diff_ * 1000) << " ms." << ::std::endl; \
    } while (0)

#else

#define measure_time(ret, op) do {              \
      (op);                                     \
      ret = 0;                                  \
  } while (0) 

#define time(op) do {                           \
        (op);                                   \
    } while (0)

#endif

#endif
