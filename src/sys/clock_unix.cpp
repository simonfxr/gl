#include "sys/clock.hpp"

#include "bl/limits.hpp"
#include "bl/string.hpp"
#include "err/err.hpp"
#include "util/string.hpp"

#include <errno.h>
#include <string.h>
#include <time.h>

namespace sys {

USE_STRING_LITERALS;

namespace {
double
getTime()
{
    timespec tv{};

    while (clock_gettime(CLOCK_MONOTONIC_RAW, &tv) == -1) {
        if (errno == EINTR) {
            errno = 0;
        } else {
            ERR(string_concat("clock_gettime failed", strerror(errno)));
            return bl::numeric_limits<double>::quiet_NaN();
        }
    }

    return double(tv.tv_sec) + double(tv.tv_nsec) * (1 / 1e9);
}
} // namespace

double
queryTimer() noexcept
{
    static const double T0 = getTime();
    return getTime() - T0;
}

void
sleep(double secs) noexcept
{
    timespec tv{}, rmtv{};
    tv.tv_sec = time_t(secs);
    tv.tv_nsec = long(secs * 1e9);

    while (nanosleep(&tv, &rmtv) == -1) {
        if (errno == EINTR) {
            errno = 0;
            tv = rmtv;
        } else {
            ERR(string_concat("sleep failed: ", strerror(errno)));
        }
    }
}

} // namespace sys
