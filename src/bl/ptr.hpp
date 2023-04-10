#ifndef BL_NON_NULL_HPP
#define BL_NON_NULL_HPP

#include "bl/ptr_traits.hpp"

namespace bl {

template<typename T>
struct ptr
{
    using _traits = ptr_traits<T>;
    using value_type = typename _traits::value_type;
    using reference_type = typename _traits::reference_type;
    static inline constexpr bool nullable = false;
    static inline constexpr bool unique = _traits::aliasing;

    ptr() = delete;
    ptr(std::nullptr_t) = delete;
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    HU_MACROLIKE HU_NONNULL_PARAMS() constexpr ptr(T p) : _p(p) {}

    HU_MACROLIKE constexpr ptr(const ptr &) = default;
    HU_MACROLIKE constexpr ptr(ptr &&) noexcept = default;
    HU_MACROLIKE constexpr ptr &operator=(const ptr &) = default;
    HU_MACROLIKE constexpr ptr &operator=(ptr &&) noexcept = default;
    HU_MACROLIKE constexpr ~ptr() = default;

    HU_MACROLIKE HU_NONNULL_PARAMS() constexpr ptr &operator=(T *p) noexcept
    {
        _p = p;
        return *this;
    }

    ptr &operator=(std::nullptr_t) = delete;

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
    HU_MACROLIKE friend bool operator==(const ptr<U> &lhs, const ptr<V> rhs)
    {
        return lhs.get() == rhs.get();
    }

private:
    T _p;
};

} // namespace bl

#endif // BL_PTR_HPP
