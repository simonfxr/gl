#ifndef BL_CONCEPTS_HPP
#define BL_CONCEPTS_HPP

#include <bl/conf.hpp>

#include <array>
#include <bit>
#include <concepts>

namespace bl {

template<typename T>
concept bitcastable = requires(const std::array<std::byte, sizeof(T)> &bytes) {
                          std::bit_cast<T>(bytes);
                      };

} // namespace bl

#endif // BL_CONCEPTS_HPP
