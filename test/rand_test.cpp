#include <bl/rand.hpp>
#include <bl/rand/uniform.hpp>

#include <math/ivec.hpp>

#include "./testmain.hpp"

using namespace bl::rand;

#define mkrng() seed_from(ctest_rand(uint64_t))

CTEST_GROUP("rand")
{
    CTEST("init")
    {
        auto x = mkrng();
        auto k = x();
        ctest_true(k > 65536);
    }

    CTEST("uniform")
    {
        auto g = mkrng();
        auto dist = uniform_distribution{ 0, 3 };
        math::ivec3 hist{};
        for (size_t i = 0; i < 10000; ++i) {
            auto x = sample(dist, g);
            ctest_true(0 <= x);
            ctest_true(x < 3);
            hist[x]++;
        }

        auto dens = hist / static_cast<float>(sum(hist));
        auto err = sum(abs(dens - 1.f / 3.f));
        ctest_true(err < 0.02);
    }
}

CTEST_MAIN()
