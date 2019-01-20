#include "err/err.hpp"
#include "util/string.hpp"

HU_NOINLINE int
foo()
{
#if HU_COMP_GNULIKE_P
    __asm__ __volatile__("" : ::"memory");
#endif
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
    ASSERT_ALWAYS(x > 0, string_concat("value: ", x));
}

int
main()
{
    bar(foo());
    baz(foo());
    bazzz(foo());
    return 0;
}
