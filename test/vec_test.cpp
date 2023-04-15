#include <math/ivec.hpp>
#include <math/uvec.hpp>
#include <sys/io.hpp>
#include <sys/sys.hpp>

PRAGMA_DIAGNOSTIC_PUSH
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wglobal-constructors"

#define CTEST_REPORT_NOT_EQUAL(expected, ...)                                  \
    ::report_not_equal((expected), (__VA_ARGS__))

#define CTEST_IMPLEMENTATION
#include <ctest.h>

using namespace math;

static void
report_not_equal(auto &&expected, auto &&rhs)
{
    static int sysinit = []() {
        sys::moduleInit();
        return 0;
    }();
    ((void) sysinit);
    sys::io::stderr() << expected << " vs " << rhs << "\n";
}

CTEST_GROUP("vec3")
{

    ivec3 v = ivec3{ 1, 2, 3 };
    ivec3 z = ivec3{ 0, 0, 0 };
    ivec3 u = ivec3{ 4, 5, 6 };

    CTEST("vector arith")
    {
        ctest_equal(v, z + v);
        ctest_equal(z, v - v);

        ctest_equal(ivec3(1, 4, 9), v * v);
        ctest_equal(ivec3(4, 2, 2), u / v);

        ctest_equal(ivec3(5, 7, 7), u | v);
        ctest_equal(ivec3(0, 0, 2), u & v);
        ctest_equal(ivec3(5, 7, 5), u ^ v);
    }

    CTEST("vector+scalar arith")
    {
        ctest_equal(v + v, 2 * v);
        ctest_equal(v + v, v * 2);
        ctest_equal(v + ivec3(broadcast_t{}, 3), v + 3);
        ctest_equal(v + ivec3(broadcast_t{}, 3), 3 + v);

        ctest_equal(-v, (-1) * v);
        ctest_equal(-v, z - v);
    }
}

CTEST_MAIN()

PRAGMA_DIAGNOSTIC_POP
