#ifndef TESTMAIN_HPP
#define TESTMAIN_HPP

#include <sys/io.hpp>
#include <sys/sys.hpp>

#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wglobal-constructors"

#define CTEST_REPORT_NOT_EQUAL(expected, ...)                                  \
    ::report_not_equal((expected), (__VA_ARGS__))

#define CTEST_IMPLEMENTATION
#include <ctest.h>

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static int sysinit = []() {
    sys::moduleInit();
    return 0;
}();

CTEST_ATTR_UNUSED static void
report_not_equal(auto &&expected, auto &&rhs)
{
    sys::io::stderr() << expected << " vs " << rhs << "\n";
}

#endif // TESTMAIN_HPP
