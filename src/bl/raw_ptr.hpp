#ifndef BL_RAW_PTR_HPP
#define BL_RAW_PTR_HPP

#include <bl/ptr_ops.hpp>
#include <bl/ptr_traits.hpp>

namespace bl {

template<typename T>
struct raw_ptr : ptr_ops<raw_ptr<T>, T, true, false>
{
    static inline constexpr bool is_nullable = true;
    static inline constexpr bool is_owned = false;
    template<typename U>
    using rebind = raw_ptr<U>;

    BL_wrapper raw_ptr() = default;
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    BL_wrapper raw_ptr(std::nullptr_t) : _p(nullptr) {}
    BL_wrapper constexpr explicit raw_ptr(T *p) : _p(p) {}

    BL_wrapper constexpr raw_ptr(const raw_ptr &) = default;
    BL_wrapper constexpr raw_ptr(raw_ptr &&rhs) noexcept : _p(rhs.release()) {}
    BL_wrapper constexpr raw_ptr &operator=(const raw_ptr &) = default;
    BL_wrapper constexpr raw_ptr &operator=(raw_ptr &&rhs) noexcept
    {
        _p = rhs.release();
        return *this;
    }
    BL_wrapper ~raw_ptr() = default;

    BL_wrapper constexpr T *release() noexcept { return exchange(_p, nullptr); }

    BL_wrapper constexpr T *get() const noexcept { return _p; }

    template<typename V>
    BL_wrapper friend constexpr bool operator==(const raw_ptr<T> &lhs,
                                                const raw_ptr<V> &rhs)
    {
        return lhs.get() == rhs.get();
    }

private:
    T *_p;
};

} // namespace bl

#endif // BL_RAW_PTR_HPP
