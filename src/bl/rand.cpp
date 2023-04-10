#include <bl/rand.hpp>

#include <array>
#include <cstring>
#include <random>

namespace bl::rand {

void
seed_bytes(std::span<std::byte> bytes) noexcept
{
    std::random_device rd;
    seed_bytes(rd, bytes);
}

} // namespace bl::rand
