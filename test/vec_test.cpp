#include <bl/rand/uniform.hpp>

#include "err/err.hpp"
#include "math/fvec.hpp"
#include "math/ivec.hpp"
#include "math/uvec.hpp"

#include "math/mat.hpp"

#include "./testmain.hpp"

using namespace math;
using namespace bl::literals;
namespace r = bl::rand;

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
        ctest_equal(v + ivec3(broadcast, 3), v + 3);
        ctest_equal(v + ivec3(broadcast, 3), 3 + v);

        ctest_equal(-v, (-1) * v);
        ctest_equal(-v, z - v);
    }

    CTEST("vec dist")
    {
        auto g = test_rng();
        auto s = r::uniform<real>();
        {
            auto vs = vec{ s, s, s };

            sys::io::stdout() << r::sample(vs, g) << "\n"_k;
            sys::io::stdout() << r::sample(vs, g) << "\n"_k;
            sys::io::stdout() << r::sample(vs, g) << "\n"_k;
            sys::io::stdout() << r::sample(vs, g) << "\n"_k;
        }
        {
            auto vs = r::uniform<vec3>(-1_r, 1_r);
            sys::io::stdout() << r::sample(vs, g) << "\n"_k;
            sys::io::stdout() << r::sample(vs, g) << "\n"_k;
            sys::io::stdout() << r::sample(vs, g) << "\n"_k;
            sys::io::stdout() << r::sample(vs, g) << "\n"_k;
        }
    }

    CTEST("vec cast")
    {
        auto v = ivec3(1, 2, 3);
        auto [w, h, d] = std::as_const(v).cast<size_t>();
        UNUSED(w);
        UNUSED(h);
        UNUSED(d);
    }
}

CTEST_MAIN()
