#include "sys/clock.hpp"

#include "err/err.hpp"
#include "sys/strerror_unix.hpp"

#include <bl/core.hpp>

#include <cerrno>
#include <cmath>
#include <cstring>
#include <ctime>
#include <string>

namespace sys {

namespace {
struct timespec
getTime()
{
    struct timespec tv
    {};
    while (clock_gettime(CLOCK_MONOTONIC_RAW, &tv) < 0) {
        auto e = bl::exchange(errno, 0);
        if (e == EINTR)
            continue;
        FATAL_ERR(std::string("clock_gettime failed") +
                  strerror_errno().data());
    }

    return tv;
}
} // namespace

double
queryTimer() noexcept
{
    static const timespec t0 = getTime();
    auto t1 = getTime();
    auto s = t1.tv_sec - t0.tv_sec;
    auto n = t1.tv_nsec - t0.tv_nsec;
    if (n < 0) {
        n += 1'000'000'000;
        s--;
    }
    return static_cast<double>(s) + static_cast<double>(n) * 1e-9;
}

void
sleep(double secs) noexcept
{
    timespec tv{};
    tv.tv_sec = static_cast<decltype(tv.tv_sec)>(secs);
    tv.tv_nsec = static_cast<decltype(tv.tv_nsec)>(
      (secs - static_cast<double>(tv.tv_sec)) * 1e9);

    timespec rmtv{};
    while (nanosleep(&tv, &rmtv) < 0) {
        auto e = bl::exchange(errno, 0);
        if (e == EINTR) {
            tv = rmtv;
            continue;
        }
        FATAL_ERR(std::string("sleep failed: ") + strerror_errno(e).data());
    }
}

} // namespace sys
