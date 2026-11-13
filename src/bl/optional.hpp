#ifndef BL_OPTIONAL_HPP
#define BL_OPTIONAL_HPP

#include <bl/conf.hpp>

#include <bl/core.hpp>
#include <bl/type_traits.hpp>

namespace bl {

inline constexpr struct nullopt_t
{
} nullopt{};

template<typename T>
struct optional
{
    static_assert(!std::is_reference_v<T>);

    constexpr optional() = default;

    constexpr /* implicit */ optional(nullopt_t) noexcept {}

    template<typename U = T>
        requires(std::is_same_v<std::decay_t<U>, T>)
    constexpr /* implicit */ optional(U &&val) : _present{ true }
    {
        bl::construct_at(place(), bl_fwd(val));
    }

    constexpr optional(const optional &) noexcept
        requires(std::is_trivially_copy_constructible_v<T>)
    = default;

    constexpr optional(const optional &rhs) noexcept(
      std::is_nothrow_copy_constructible_v<T>)
        requires(!std::is_trivially_copy_constructible_v<T>)
      : _present(rhs._present)
    {
        if (_present)
            bl::construct_at(place(), *rhs.place());
    }

    constexpr optional(optional &&rhs) noexcept
        requires(std::is_trivially_move_constructible_v<T>)
    = default;

    constexpr optional(optional &&rhs) noexcept(
      std::is_nothrow_move_constructible_v<T>)
        requires(!std::is_trivially_move_constructible_v<T>)
      : _present(bl::exchange(rhs._present, false))
    {
        if (_present) {
            bl::construct_at(place(), bl::move(*rhs.place()));
            bl::destroy_at(rhs.place());
        }
    }

    static inline constexpr bool is_trivially_move_assignable =
      std::is_trivially_move_assignable_v<T> &&
      std::is_trivially_destructible_v<T> &&
      std::is_trivially_move_constructible_v<T>;

    constexpr optional &operator=(optional &&) noexcept
        requires(is_trivially_move_assignable)
    = default;

    constexpr optional &operator=(optional &&rhs) noexcept(
      std::is_nothrow_move_assignable_v<T>)
        requires(!is_trivially_move_assignable)
    {
        if (_present && rhs._present) {
            *place() = bl::move(*rhs.place());
            bl::destroy_at(rhs.place());
            rhs._present = false;
        } else if (!_present && rhs._present) {
            bl::construct_at(place(), bl::move(*rhs.place()));
            bl::destroy_at(rhs.place());
            _present = bl::exchange(rhs._present, false);
        } else if (_present) {
            bl::destroy_at(place());
            _present = false;
        }
        return *this;
    }

    static inline constexpr bool is_trivially_copy_assignable =
      std::is_trivially_copy_assignable_v<T> &&
      std::is_trivially_destructible_v<T> &&
      std::is_trivially_copy_constructible_v<T>;

    constexpr optional &operator=(const optional &) noexcept
        requires(is_trivially_copy_assignable)
    = default;

    constexpr optional &operator=(const optional &rhs) noexcept(
      std::is_nothrow_copy_assignable_v<T>)
        requires(!is_trivially_copy_assignable)
    {
        *this = optional{ rhs };
        return *this;
    }

    constexpr optional &operator=(bl::nullopt_t) noexcept(
      std::is_trivially_destructible_v<T>)
    {
        if (_present) {
            bl::destroy_at(place());
            _present = false;
        }
        return *this;
    }

    ~optional()
        requires(std::is_trivially_destructible_v<T>)
    = default;

    ~optional()
        requires(!std::is_trivially_destructible_v<T>)
    {
        *this = nullopt;
    }

    [[nodiscard]] BL_inline const T &operator*() const & noexcept
    {
        return *place();
    }
    [[nodiscard]] BL_inline T &operator*() & noexcept { return *place(); }
    [[nodiscard]] BL_inline T &&operator*() && noexcept
    {
        return bl::move(*place());
    }

    [[nodiscard]] BL_inline const T *operator->() const noexcept
    {
        return place();
    }
    [[nodiscard]] BL_inline T *operator->() noexcept { return place(); }

    [[nodiscard]] BL_inline explicit operator bool() const noexcept
    {
        return _present;
    }

    template<typename... Args>
    constexpr T &emplace(Args &&...args)
    {
        *this = nullopt;
        bl::construct_at(place(), bl_fwd(args)...);
        _present = true;
        return *place();
    }

private:
    [[nodiscard]] BL_wrapper const T *place() const noexcept
    {
        if constexpr (trivial_storage) {
            return bl::addressof(_store);
        } else {
            return bl::addressof(_store._place);
        }
    }

    [[nodiscard]] BL_wrapper T *place() noexcept
    {
        if constexpr (trivial_storage) {
            return bl::addressof(_store);
        } else {
            return bl::addressof(_store._place);
        }
    }

    // NOLINTNEXTLINE(hicpp-special-member-functions,cppcoreguidelines-special-member-functions)
    union default_storage
    {
        T _place;
        // NOLINTNEXTLINE(hicpp-use-equals-default,modernize-use-equals-default)
        BL_wrapper constexpr default_storage() noexcept {}
        // NOLINTNEXTLINE(hicpp-use-equals-default,modernize-use-equals-default)
        BL_wrapper constexpr ~default_storage() {}
    };

    static inline constexpr bool trivial_storage =
      std::is_trivially_default_constructible_v<T> &&
      std::is_trivially_destructible_v<T>;

    using storage = std::conditional_t<trivial_storage, T, default_storage>;

    storage _store{};
    [[no_unique_address]] bool _present{};
};

} // namespace bl

#endif // BL_OPTIONAL_HPP
