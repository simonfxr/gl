#include "sys/clock.hpp"

#include "err/err.hpp"

#include <cerrno>
#include <cmath>
#include <cstring>
#include <ctime>
#include <string>

namespace sys {

namespace {
inline constexpr auto SECONDS_PER_NANOSECOND = 1e-9;
inline constexpr auto NANOSECONDS_PER_SECOND = 1e9;
} // namespace

namespace {
double
getTime()
{
    timespec tv{};

    while (clock_gettime(CLOCK_MONOTONIC_RAW, &tv) == -1) {
        if (errno == EINTR) {
            errno = 0;
        } else {
            ERR(std::string("clock_gettime failed") + strerror(errno));
            return NAN;
        }
    }

    return double(tv.tv_sec) + double(tv.tv_nsec) * SECONDS_PER_NANOSECOND;
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
    timespec tv{};
    tv.tv_sec = static_cast<decltype(tv.tv_sec)>(secs);
    tv.tv_nsec = static_cast<decltype(tv.tv_nsec)>((secs - tv.tv_sec) *
                                                   NANOSECONDS_PER_SECOND);

    timespec rmtv{};
    while (nanosleep(&tv, &rmtv) == -1) {
        if (errno == EINTR) {
            errno = 0;
            tv = rmtv;
        } else {
            ERR(std::string("sleep failed: ") + strerror(errno));
        }
    }
}

} // namespace sys
