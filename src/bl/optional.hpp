#ifndef BL_OPTIONAL_HPP
#define BL_OPTIONAL_HPP

#include "bl/core.hpp"
#include "bl/new.hpp"
#include "bl/type_traits.hpp"
namespace bl {

struct nullopt_t
{};

inline constexpr nullopt_t nullopt = nullopt_t{};

template<typename T>
struct optional
{
    static_assert(is_nothrow_destructible_v<T>);

    constexpr optional() noexcept : _dummy() {}

    constexpr optional(const nullopt_t &) noexcept : optional() {}

    optional(const T &value) : _value(value), _present(true) {}

    optional(T &&value) : _value(bl::move(value)), _present(true) {}

    template<typename U = T>
    optional(optional<U> &&opt) : optional()
    {
        assign(bl::move(opt));
    }

    template<typename U = T>
    optional(const optional<U> &opt) : optional()
    {
        assign(opt);
    }

    ~optional() { clear(); }

    template<typename U = T>
    optional &operator=(const optional<U> &rhs)
    {
        return assign(rhs);
    }

    template<typename U = T>
    optional &operator=(optional<U> &&rhs)
    {
        return assign(bl::move(rhs));
    }

    void clear() noexcept
    {
        if (_present)
            _value.~T();
        _present = false;
    }

    FORCE_INLINE const T &value() const & { return _value; }
    FORCE_INLINE const T &&value() const && { return bl::move(_value); }
    FORCE_INLINE T &value() & { return _value; }
    FORCE_INLINE T &&value() && { return bl::move(_value); }

    FORCE_INLINE const T *operator->() const { return addressof(_value); }
    FORCE_INLINE T *operator->() { return addressof(_value); }

    FORCE_INLINE const T &operator*() const & { return value(); }
    FORCE_INLINE const T &&operator*() const && { return bl::move(value()); }
    FORCE_INLINE T &operator*() & { return value(); }
    FORCE_INLINE T &&operator*() && { return bl::move(value()); }

    FORCE_INLINE constexpr explicit operator bool() const noexcept
    {
        return _present;
    }

private:
    template<typename U>
    void assign_val(U &&val)
    {
        if (_present) {
            _value = bl::forward<U>(val);
        } else {
            new (addressof(_value)) T(bl::forward<U>(val));
        }
    }

    template<typename U>
    optional &assign(const optional<U> &r)
    {
        if (r._present)
            assign_val(r._value);
        else
            clear();
        return *this;
    }

    template<typename U>
    optional &assign(const optional<U> &&r)
    {
        if (r._present)
            assign_val(bl::move(r._value));
        else
            clear();
        return *this;
    }

    union
    {
        T _value;
        char _dummy;
    };
    bool _present{};
};

} // namespace bl

#endif
