#ifndef BL_RELATIVE_PTR_HPP
#define BL_RELATIVE_PTR_HPP

#include <bl/concepts.hpp>
#include <bl/ptr_ops.hpp>
#include <bl/ptr_traits.hpp>

namespace bl {

template<typename T, std::signed_integral Rep = std::intptr_t>
struct relative_ptr
  : ptr_ops<relative_ptr<T, Rep>, T, std::is_same_v<Rep, std::intptr_t>, false>
{
    using element_type = T;
    static inline constexpr bool is_nullable =
      std::is_same_v<Rep, std::intptr_t>;
    static inline constexpr bool is_owned = false;

    template<typename U>
    using rebind = relative_ptr<U, Rep>;

    BL_wrapper relative_ptr()
        requires(is_nullable)
      : relative_ptr(nullptr)
    {}

    relative_ptr()
        requires(!is_nullable)
    = delete;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    BL_wrapper relative_ptr(std::nullptr_t)
        requires(is_nullable)
      : relative_ptr(static_cast<T *>(nullptr))
    {}

    relative_ptr(std::nullptr_t)
        requires(!is_nullable)
    = delete;

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    BL_wrapper relative_ptr(T *p)
        requires(is_nullable)
      : _p(relative_to(this, p))
    {}

    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    BL_wrapper HU_NONNULL_PARAMS() relative_ptr(T *p)
        requires(!is_nullable)
      : _p(relative_to(this, p))
    {}

    BL_wrapper relative_ptr(const relative_ptr &rhs)
      : _p(relative_to(this, rhs.get()))
    {}

    BL_wrapper relative_ptr(relative_ptr &&rhs) noexcept
      : _p(relative_to(this, rhs.get()))
    {}

    BL_wrapper relative_ptr &operator=(const relative_ptr &rhs)
    {
        operator=(rhs.get());
        return *this;
    }

    BL_wrapper relative_ptr &operator=(relative_ptr &&rhs) noexcept
    {
        operator=(rhs.get());
        return *this;
    }

    BL_wrapper ~relative_ptr() = default;

    BL_wrapper T *get_or_null() const noexcept { return absolute(this, _p); }

    BL_wrapper constexpr T *release() noexcept
        requires(is_nullable)
    {
        return exchange(this, relative_ptr{}).get();
    }

    template<typename U>
    BL_wrapper friend bool operator==(const relative_ptr<T> &lhs,
                                      const relative_ptr<U> &rhs)
    {
        return lhs.get() == rhs.get();
    }

private:
    BL_wrapper static Rep relative_to(const relative_ptr *base, T *p) noexcept
    {
        return static_cast<Rep>(static_cast<char *>(static_cast<const T *>(p)) -
                                static_cast<const char *>(base));
    }

    BL_wrapper static T *absolute(const relative_ptr *base, Rep offset) noexcept
    {
        static_cast<T *>(static_cast<void *>(
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
          const_cast<char *>(static_cast<const char *>(base) +
                             static_cast<std::intptr_t>(offset))));
    }

    Rep _p;
};

} // namespace bl

#endif // BL_RELATIVE_PTR_HPP
