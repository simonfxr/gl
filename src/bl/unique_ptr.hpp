#ifndef BL_UNIQUE_PTR_HPP
#define BL_UNIQUE_PTR_HPP

#include "bl/core.hpp"
#include "bl/type_traits.hpp"

namespace bl {

template<typename T>
struct DefaultDeleter
{
    void operator()(T *p) noexcept { delete p; }
};

template<typename T, typename Deleter = DefaultDeleter<T>>
struct unique_ptr
{
    static_assert(std::is_empty_v<Deleter>,
                  "only stateless deleters are supported");
    static_assert(std::is_nothrow_default_constructible_v<Deleter>);
    static_assert(!std::is_array_v<T>,
                  "unique_ptr with array is not supported");

    constexpr unique_ptr() = default;

    template<typename U>
    constexpr unique_ptr(U *x) : _ptr(x)
    {}

    unique_ptr(const unique_ptr &) = delete;

    constexpr unique_ptr(unique_ptr &&rhs)
      : _ptr(::bl::exchange(rhs._ptr, nullptr))
    {}

    template<typename U, typename D>
    constexpr unique_ptr(unique_ptr<U, D> &&rhs)
      : _ptr(exchange(rhs._ptr, nullptr))
    {}

    ~unique_ptr() { reset(); }

    unique_ptr &operator=(const unique_ptr &) = delete;

    constexpr unique_ptr &operator=(unique_ptr &&rhs) noexcept
    {
        reset();
        _ptr = exchange(rhs._ptr, nullptr);
        return *this;
    }

    template<typename U, typename D>
    constexpr unique_ptr &operator=(unique_ptr<U, D> &&rhs) noexcept
    {
        reset();
        _ptr = exchange(rhs._ptr, nullptr);
        return *this;
    }

    FORCE_INLINE constexpr T *release() noexcept
    {
        return exchange(_ptr, nullptr);
    }

    template<typename U = T>
    void reset(U *p = nullptr) noexcept
    {
        static_assert(noexcept(Deleter{}(_ptr)));
        if (_ptr)
            Deleter{}(_ptr);
        _ptr = p;
    }

    FORCE_INLINE constexpr T *get() const noexcept { return _ptr; }

    FORCE_INLINE constexpr explicit operator bool() const
    {
        return _ptr != nullptr;
    }

    FORCE_INLINE decltype(auto) operator*() const noexcept { return *_ptr; }
    FORCE_INLINE T *operator->() const noexcept { return _ptr; }

private:
    T *_ptr = nullptr;
    template<typename U, typename D>
    friend struct unique_ptr;

public:
#define DEF_REL_OP(ta, tb, op, px, py)                                         \
    template<typename A, typename B>                                           \
    friend bool operator op(const ta x, const tb y) noexcept                   \
    {                                                                          \
        return px op py;                                                       \
    }

#define DEF_REL_OPS(ta, tb, px, py)                                            \
    DEF_REL_OP(ta, tb, ==, px, py)                                             \
    DEF_REL_OP(ta, tb, !=, px, py)                                             \
    DEF_REL_OP(ta, tb, <, px, py)                                              \
    DEF_REL_OP(ta, tb, <=, px, py)                                             \
    DEF_REL_OP(ta, tb, >, px, py)                                              \
    DEF_REL_OP(ta, tb, >=, px, py)

    DEF_REL_OPS(unique_ptr<A> &, unique_ptr<B>, x.get(), y.get())
    DEF_REL_OPS(unique_ptr<A> &, B *, x.get(), y)
    DEF_REL_OPS(A *, unique_ptr<B> &, x, y.get())

#undef DEF_REL_OPS
#undef DEF_REL_OP
};

template<typename T, typename... Args>
FORCE_INLINE inline unique_ptr<T>
make_unique(Args &&... args)
{
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace bl

#endif
