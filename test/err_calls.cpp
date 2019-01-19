#include "err/err.hpp"
#include "util/string_utils.hpp"

HU_NOINLINE int
foo()
{
    __asm__ __volatile__("" : ::"memory");
    return 42;
}

void
bar(int x)
{
    ASSERT_ALWAYS(x > 0);
    foo();
}

void
baz(int x)
{
    ASSERT_ALWAYS(x > 0, "x is non positive");
}

void
bazzz(int x)
{
    ASSERT_ALWAYS(x > 0, x % 2 ? "odd" : "even");
}

int
main()
{
    bar(foo());
    baz(foo());
    bazzz(foo());
    return 0;
}
