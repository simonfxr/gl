#ifndef BL_NON_NULL_HPP
#define BL_NON_NULL_HPP

#include <bl/ptr_ops.hpp>
#include <bl/ptr_traits.hpp>

namespace bl {

template<a_ptr T>
struct non_null
  : public ptr_ops<non_null<T>,
                   typename ptr_traits<T>::element_type,
                   false,
                   ptr_traits<T>::is_owned>
{
    using _traits = ptr_traits<T>;
    using element_type = typename _traits::element_type;
    static inline constexpr bool is_nullable = false;
    static inline constexpr bool is_owned = _traits::is_owned;

    template<typename U>
    using rebind = non_null<typename _traits::template rebind<U>>;

    non_null() = delete;
    non_null(std::nullptr_t) = delete;
    HU_NONNULL_PARAMS()
    BL_wrapper explicit constexpr non_null(element_type *p) : _p(p) {}

    BL_wrapper constexpr non_null(const non_null &) = default;
    BL_wrapper constexpr non_null(non_null &&) noexcept = default;
    BL_wrapper constexpr non_null &operator=(const non_null &) = default;
    BL_wrapper constexpr non_null &operator=(non_null &&) noexcept = default;
    BL_wrapper constexpr ~non_null() = default;

    BL_wrapper constexpr element_type *release() &&noexcept
    {
        auto tmp = bl_mov(*this);
        return bl_mov(tmp._p).release();
    }

    BL_wrapper constexpr element_type *get_or_null() const noexcept
    {
        return _p.get_or_null();
    }

    template<typename U>
    BL_wrapper friend bool operator==(const non_null<T> &lhs,
                                      const non_null<U> &rhs)
    {
        return lhs.get() == rhs.get();
    }

private:
    T _p;
};

} // namespace bl

#endif // BL_NON_NULL_HPP
