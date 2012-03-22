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

const double INVERSE_FREQ = initFreq();

double getTime() {
	LARGE_INTEGER t;
	bool ok = QueryPerformanceCounter(&t) == TRUE;
	ASSERT_MSG(ok, "QueryPerformanceCounter() failed");
	return t.QuadPart * INVERSE_FREQ;
}

}

double queryTimer() {
	static const double T0 = getTime();
	return getTime() - T0;
}

void sleep(double secs) {
	DWORD millis = DWORD(secs * 1000);
	double wakeup = queryTimer() + secs;
	if (millis > 0)
		Sleep(millis);
	while (queryTimer() < wakeup)
		;
}

} // namespace sys
