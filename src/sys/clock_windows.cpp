#include "sys/clock.hpp"

#include <err/err.hpp>

#include <Windows.h>

namespace sys {
namespace {

double
initFreq()
{
    LARGE_INTEGER freq;
    bool ok = QueryPerformanceFrequency(&freq) == TRUE;
    ASSERT_MSG(ok && freq.QuadPart != 0, "QueryPerformanceFrequency() failed");
    return 1.0 / double(freq.QuadPart);
}

double
getTime(double invFreq)
{
    LARGE_INTEGER t;
    bool ok = QueryPerformanceCounter(&t) == TRUE;
    ASSERT_MSG(ok, "QueryPerformanceCounter() failed");
    return t.QuadPart * invFreq;
}

struct Clock
{
    const double INVERSE_FREQ;
    const double T0;

    Clock() : INVERSE_FREQ(initFreq()), T0(getTime(INVERSE_FREQ)) {}
};
} // namespace

double
queryTimer() noexcept
{
    static const Clock clock;
    return getTime(clock.INVERSE_FREQ) - clock.T0;
}

void
sleep(double secs) noexcept
{
    DWORD millis = DWORD(secs * 1000);
    double wakeup = queryTimer() + secs;
    if (millis > 0)
        Sleep(millis);
    while (queryTimer() < wakeup)
        ;
}
} // namespace sys
