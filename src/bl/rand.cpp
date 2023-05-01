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

random_device::result_type
random_device::operator()() noexcept
{
    struct
    {
        std::random_device &rd;
        using result_type = decltype(rd());
        result_type operator()() noexcept { return rd(); }
    } g{ _p->rd };
    std::array<byte, sizeof(random_device::result_type)> sbytes{};
    bl::rand::seed_bytes(g, sbytes);
    return bit_cast<random_device::result_type>(sbytes);
}

void
random_device::seed_bytes(bl::mut_byte_span bytes) noexcept
{
    bl::rand::seed_bytes(*this, bytes);
}

} // namespace bl::rand
