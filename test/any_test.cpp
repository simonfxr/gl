#include <bl/any_unique.hpp>
#include <bl/tag_invoke.hpp>
#include <bl/type_traits.hpp>

#include <bl/string_view.hpp>
#include <sys/io.hpp>
#include <sys/io/Stream.hpp>

namespace test {

using namespace bl::literals;

inline constexpr struct foo_f
{
    BL_DEF_TAG_CPO_OPERATOR(foo_f)
} foo;

struct bar
{
    friend int tag_invoke(foo_f, bar &)
    {
        sys::io::stdout() << "bar"_k
                          << "\n"_k;
        return 42;
    }
};

using any_foo = bl::any_unique_t<bl::overload<int(bl::self &)>(foo)>;
using any_foo_ref = bl::any_ref_t<bl::overload<int(bl::self &)>(foo)>;

} // namespace test

int
main()
{
    using namespace test;
    bar b;
    foo(b);
    any_foo af = b;
    foo(af);
    any_foo_ref afr = b;
    foo(afr);
}
