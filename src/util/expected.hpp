#ifndef UTIL_RESULT_HPP
#define UTIL_RESULT_HPP

#include "defs.h"

#include <compare>
#include <type_traits>
#include <utility>

#define USE_STD_EXPECTED 1

#if defined(USE_STD_EXPECTED) && hu_has_include(<expected>)

#    include <expected>
namespace util {
using unexpect_t = std::unexpect_t;
inline constexpr unexpect_t unexpect = std::unexpect;

template<typename E>
using unexpected = std::unexpected<E>;

template<typename T, typename E>
using expected = std::expected<T, E>;

} // namespace util

#else
#    include <variant>

namespace util {

template<typename E>
struct unexpected
{
    E _err;

    constexpr decltype(auto) error() const &noexcept { return _err; }
    constexpr decltype(auto) error() const && = delete;
    constexpr decltype(auto) error() &noexcept { return _err; }
    constexpr decltype(auto) error() &&noexcept { return std::move(_err); }

    constexpr auto friend operator<=>(const unexpected &lhs,
                                      const unexpected &rhs) = default;
};

template<class E>
unexpected(E) -> unexpected<E>;

struct unexpect_t
{
    explicit unexpect_t() = default;
};

inline constexpr unexpect_t unexpect{};

template<typename T, typename E>
class expected : private std::variant<T, unexpected<E>>
{
    using base_t = std::variant<T, unexpected<E>>;

public:
    using std::variant<T, unexpected<E>>::variant;

    friend constexpr auto operator<=>(const expected &lhs,
                                      const expected &rhs) = default;

    constexpr expected() noexcept : base_t{ T{} } {}

    template<class U = T>
    constexpr explicit(!std::is_convertible_v<U, T>) expected(U &&v)
      : base_t{ std::forward<U>(v) }
    {}

    template<class G>
    constexpr explicit(!std::is_convertible_v<const G &, E>)
      expected(const unexpected<G> &e)
      : base_t{ unexpected<E>{ e.error() } }
    {}

    template<class G>
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G> &&e)
      : base_t{ unexpected<E>{ std::move(e).error() } }
    {}

    constexpr explicit operator bool() const noexcept
    {
        return base_t::index() == 0;
    }

    template<class... Args>
    constexpr explicit expected(unexpect_t, Args &&...args)
      : base_t{ unexpected{ std::forward<Args>(args)... } }
    {}

    constexpr decltype(auto) value() const &noexcept
    {
        return std::get<0>(*this);
    }
    constexpr decltype(auto) value() const &&noexcept = delete;
    constexpr decltype(auto) value() &noexcept { return std::get<0>(*this); }
    constexpr decltype(auto) value() &&noexcept
    {
        return std::get<0>(std::move(*this));
    }

    constexpr decltype(auto) error() const &noexcept
    {
        return std::get<1>(*this).error();
    }

    constexpr decltype(auto) error() const &&noexcept
    {
        return std::get<1>(*this).error();
    }

    constexpr decltype(auto) error() &noexcept
    {
        return std::get<1>(*this).error();
    }

    constexpr E error() &&noexcept
    {
        return std::get<1>(std::move(*this)).error();
    }

    constexpr const T *operator->() const noexcept { return &value(); }
    constexpr T *operator->() noexcept { return &value(); }

    constexpr const T &operator*() const &noexcept { return value(); }
    constexpr T &&operator*() &&noexcept { return std::move(*this).value(); }

    constexpr bool has_value() const noexcept
    {
        return static_cast<bool>(*this);
    }

    template<class U>
    constexpr T value_or(U &&default_value) const &
    {
        if (!*this)
            return std::forward<U>(default_value);
        return value();
    }

    template<class U>
    constexpr T value_or(U &&default_value) &&
    {
        if (!*this)
            return std::forward<U>(default_value);
        return std::move(*this).value();
    }
};

} // namespace util
#endif // !USE_STD_EXPECTED

#endif // UTIL_RESULT_HPP
