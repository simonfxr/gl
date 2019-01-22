#include "sys/clock.hpp"

#include "err/err.hpp"

#include "bl/string.hpp"
#include <cerrno>
#include <cmath>
#include <cstring>
#include <ctime>

namespace sys {

namespace {
double
getTime()
{
    timespec tv{};

    while (clock_gettime(CLOCK_MONOTONIC_RAW, &tv) == -1) {
        if (errno == EINTR) {
            errno = 0;
        } else {
            ERR(bl::string("clock_gettime failed") + strerror(errno));
            return NAN;
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
            ERR(bl::string("sleep failed: ") + strerror(errno));
        }
    }
}

} // namespace sys
