#ifndef BL_OWNED_HPP
#define BL_OWNED_HPP

#include <bl/ptr_ops.hpp>
#include <bl/swap.hpp>

#include <memory>

namespace bl {

template<a_ptr T,
         typename Deleter =
           std::default_delete<typename ptr_traits<T>::element_type>>
struct owned
  : ptr_ops<owned<T, Deleter>,
            typename ptr_traits<T>::element_type,
            ptr_traits<T>::nullable,
            true>
{
    using _traits = ptr_traits<T>;
    using element_type = typename _traits::element_type;
    static inline constexpr bool nullable = _traits::nullable;
    static inline constexpr bool unique = true;

    BL_wrapper owned()
        requires(nullable)
      : _p(nullptr), _d{}
    {}

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    BL_wrapper owned(std::nullptr_t)
        requires(nullable)
      : _p(nullptr), _d{}
    {}

    BL_wrapper explicit owned(element_type *p)
        requires(nullable)
      : _p(p), _d{}
    {}

    HU_NONNULL_PARAMS()
    BL_wrapper explicit owned(element_type *p)
        requires(!nullable)
      : _p(p), _d{}
    {}

    BL_wrapper owned(element_type *p, Deleter d)
        requires(nullable)
      : _p(p), _d{ move(d) }
    {}

    HU_NONNULL_PARAMS(2)
    BL_wrapper owned(element_type *p, Deleter d)
        requires(!nullable)
      : _p(p), _d{ move(d) }
    {}

    owned(const owned &rhs) = delete;

    BL_wrapper owned(owned &&rhs) noexcept = default;

    owned &operator=(const owned &) = delete;

    owned &operator=(owned &&rhs) noexcept
    {
        auto old = bl::move(*this);
        bl::swap(_p, rhs._p);
        bl::swap(_d, rhs._d);
        return *this;
    }

    BL_wrapper constexpr element_type *release() noexcept
        requires(nullable)
    {}

    ~owned()
    {
        if (auto p = get())
            _d(p);
    }

    BL_wrapper constexpr element_type *get() const noexcept { return _p.get(); }

private:
    T _p;
    // NOLINTNEXTLINE(modernize-use-default-member-init)
    [[no_unique_address]] Deleter _d;
};

} // namespace bl

#endif // BL_OWNED_HPP
