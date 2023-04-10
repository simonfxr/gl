#ifndef BL_NON_NULL_HPP
#define BL_NON_NULL_HPP

#include <bl/ptr_traits.hpp>

namespace bl {

template<typename T>
struct non_null
{
    using _traits = ptr_traits<T>;
    using value_type = typename _traits::value_type;
    using reference_type = typename _traits::reference_type;
    static inline constexpr bool nullable = false;
    static inline constexpr bool unique = _traits::aliasing;

    non_null() = delete;
    non_null(std::nullptr_t) = delete;
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    HU_MACROLIKE HU_NONNULL_PARAMS() constexpr non_null(T p) : _p(p) {}

    HU_MACROLIKE constexpr non_null(const non_null &) = default;
    HU_MACROLIKE constexpr non_null(non_null &&) noexcept = default;
    HU_MACROLIKE constexpr non_null &operator=(const non_null &) = default;
    HU_MACROLIKE constexpr non_null &operator=(non_null &&) noexcept = default;
    HU_MACROLIKE ~non_null() = default;

    HU_MACROLIKE HU_NONNULL_PARAMS() constexpr non_null &operator=(
      T *p) noexcept
    {
        _p = p;
        return *this;
    }

    non_null &operator=(std::nullptr_t) = delete;

    HU_MACROLIKE HU_NONNULL_PARAMS() constexpr void reset(T *p) noexcept
    {
        *this = p;
    }

    void reset(std::nullptr_t) = delete;

    HU_MACROLIKE constexpr T get() const noexcept
    {
        auto v = _p;
        ASSUME(v);
        return v;
    }

    HU_MACROLIKE constexpr T operator*() const noexcept { return get(); }

    HU_MACROLIKE constexpr T operator->() const noexcept { return get(); }

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    HU_MACROLIKE constexpr operator T() noexcept { return get(); }

    HU_MACROLIKE constexpr explicit operator bool() noexcept { return true; }

    template<typename U, typename V>
    HU_MACROLIKE friend bool operator==(const non_null<U> &lhs,
                                        const non_null<V> rhs)
    {
        return lhs.get() == rhs.get();
    }

private:
    T _p;
};

} // namespace bl

#endif // BL_NON_NULL_HPP
