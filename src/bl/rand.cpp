#include <bl/rand.hpp>

#include <array>
#include <cstring>
#include <random>

namespace bl::rand {

struct random_device::Impl
{
    std::random_device rd;
};

random_device::random_device() noexcept : _p{ new Impl } {}
random_device::random_device(const random_device &) noexcept : random_device()
{}
random_device::random_device(random_device &&rhs) noexcept = default;
random_device::~random_device() = default;

random_device &
random_device::operator=(const random_device &rhs) noexcept
{
    *this = random_device(rhs);
    return *this;
}

random_device &
random_device::operator=(random_device &&rhs) noexcept = default;

void
random_device::seed_bytes(std::span<std::byte> bytes) noexcept
{
    bl::rand::seed_bytes(*this, bytes);
}

} // namespace bl::rand
