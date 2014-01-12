#include "sys/clock.hpp"

#include "err/err.hpp"

#include <time.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <math.h>

namespace sys {

static double getTime() {
    struct timespec tv;
    
    while (clock_gettime(CLOCK_MONOTONIC_RAW, &tv) == -1) {
        if (errno == EINTR) {
            errno = 0;
        } else {
            ERR(std::string("clock_gettime failed") + strerror(errno));
            return NAN;
        }
    }
    
    return double(tv.tv_sec) + double(tv.tv_nsec) * (1 / 1e9);
}

double queryTimer() {
    static double T0 = getTime();
    return getTime() - T0;
}

void sleep(double secs) {
    struct timespec tv, rmtv;
    tv.tv_sec = time_t(secs);
    tv.tv_nsec = long(secs * 1e9);

    while (nanosleep(&tv, &rmtv) == -1) {
        if (errno == EINTR) {
            errno = 0;
            tv = rmtv;
        } else {
            ERR(std::string("sleep failed: ") + strerror(errno));
        }
    }
}

}
