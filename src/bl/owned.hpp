#ifndef BL_OWNED_HPP
#define BL_OWNED_HPP

#include <bl/new.hpp>
#include <bl/ptr_ops.hpp>
#include <bl/swap.hpp>

namespace bl {

template<a_ptr T, typename Deleter = default_deleter>
struct owned
  : ptr_ops<owned<T, Deleter>,
            typename ptr_traits<T>::element_type,
            ptr_traits<T>::is_nullable,
            true>
{
    using Base = ptr_ops<owned<T, Deleter>,
                         typename ptr_traits<T>::element_type,
                         ptr_traits<T>::is_nullable,
                         true>;
    using _traits = ptr_traits<T>;
    using element_type = typename _traits::element_type;
    static inline constexpr bool is_nullable = _traits::is_nullable;
    static inline constexpr bool is_owned = true;

    BL_wrapper owned()
        requires(is_nullable)
      : _p(nullptr), _d{}
    {}

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    BL_wrapper owned(std::nullptr_t)
        requires(is_nullable)
      : _p(nullptr), _d{}
    {}

    BL_wrapper explicit owned(element_type *p)
        requires(is_nullable)
      : _p(p), _d{}
    {}

    HU_NONNULL_PARAMS()
    BL_wrapper explicit owned(element_type *p)
        requires(!is_nullable)
      : _p(p), _d{}
    {}

    BL_wrapper owned(element_type *p, Deleter d)
        requires(is_nullable)
      : _p(p), _d{ move(d) }
    {}

    HU_NONNULL_PARAMS(2)
    BL_wrapper owned(element_type *p, Deleter d)
        requires(!is_nullable)
      : _p(p), _d{ move(d) }
    {}

    owned(const owned &rhs) = delete;

    BL_wrapper owned(owned &&rhs) noexcept = default;

    using Base::operator=;

    owned &operator=(const owned &) = delete;

    owned &operator=(owned &&rhs) noexcept
    {
        auto old = bl::move(*this);
        bl::swap(_p, rhs._p);
        bl::swap(_d, rhs._d);
        return *this;
    }

    BL_wrapper constexpr element_type *release() &noexcept
        requires(is_nullable)
    {}

    BL_wrapper constexpr element_type *release() &&noexcept
    {
        auto tmp = bl_mov(*this);
        return bl_mov(tmp._p).release();
    }

    BL_wrapper constexpr element_type *get_or_null() const noexcept
    {
        return _p.get_or_null();
    }

    ~owned()
    {
        if (auto p = this->get_or_null())
            _d(p);
    }

private:
    T _p;
    // NOLINTNEXTLINE(modernize-use-default-member-init)
    [[no_unique_address]] Deleter _d;
};

} // namespace bl

#endif // BL_OWNED_HPP
