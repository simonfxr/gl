#include "sys/clock.hpp"

#include <err/err.hpp>

#include <windows.h>

namespace sys {

namespace {

double initFreq() {
    LARGE_INTEGER freq;
    bool ok = QueryPerformanceFrequency(&freq) == TRUE;
    ASSERT_MSG(ok && freq.QuadPart != 0, "QueryPerformanceFrequency() failed");
    return 1.0 / double(freq.QuadPart);
}

double INVERSE_FREQ = 0;
double T0 = -1.0;
    
double getTime() {
    if (INVERSE_FREQ == 0)
	INVERSE_FREQ = initFreq();
    LARGE_INTEGER t;
    bool ok = QueryPerformanceCounter(&t) == TRUE;
    ASSERT_MSG(ok, "QueryPerformanceCounter() failed");
    return t.QuadPart * INVERSE_FREQ;
}

}

double queryTimer() {
    if (T0 < 0)
	T0 = getTime();
    return getTime() - T0;
}

void sleep(double secs) {
    DWORD millis = DWORD(secs * 1000.0);
    double wakeup = queryTimer() + secs;
    if (millis > 0)
	Sleep(millis);
    while (queryTimer() < wakeup)
	;
}

} // namespace sys
