#include "sys/clock.hpp"

#include "err/err.hpp"

#include <time.h>
#include <string>
#include <string.h>
#include <errno.h>

namespace sys {

static double getTime() {
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return tv.tv_sec + tv.tv_nsec * (1 / 1e9);
}

double queryTimer() {
    static double T0 = getTime();
    return getTime() - T0;
}

void sleep(double secs) {
    struct timespec tv;
    tv.tv_sec = (time_t) secs;
    tv.tv_nsec = (long) (secs * 1e9);
    if (nanosleep(&tv, NULL) == -1)
        ERR(std::string("sleep failed: ") + strerror(errno));
}

}
