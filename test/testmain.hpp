#ifndef TESTMAIN_HPP
#define TESTMAIN_HPP

#include <bl/rand.hpp>
#include <sys/io.hpp>
#include <sys/sys.hpp>

#if HU_COMP_CLANG_P
#    pragma clang diagnostic ignored "-Wold-style-cast"
#    pragma clang diagnostic ignored "-Wglobal-constructors"
#endif

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
    using namespace bl::literals;
    sys::io::stderr() << expected << " vs "_k << rhs << "\n"_k;
}

#define test_rng() ::bl::rand::seed_from(ctest_rand(uint64_t))

#endif // TESTMAIN_HPP
