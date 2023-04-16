#ifndef BL_NON_COPYMOVE_HPP
#define BL_NON_COPYMOVE_HPP

#include <bl/conf.hpp>

namespace bl {

#define BL_DISABLE_COPY(T)                                                     \
    T(const T &) = delete;                                                     \
    T &operator=(const T &) = delete

struct non_copyable
{
    constexpr non_copyable() = default;
    constexpr non_copyable(non_copyable &&) = default;
    constexpr non_copyable &operator=(non_copyable &&) noexcept = default;
    constexpr ~non_copyable() = default;
    BL_DISABLE_COPY(non_copyable);
};

#define BL_DISABLE_MOVE(T)                                                     \
    T(T &&) = delete;                                                          \
    T &operator=(T &&) = delete

struct non_moveable
{
    constexpr non_moveable() = default;
    constexpr non_moveable(const non_moveable &) = default;
    constexpr non_moveable &operator=(const non_moveable &) = default;
    constexpr ~non_moveable() = default;
    BL_DISABLE_MOVE(non_moveable);
};

#define BL_DISABLE_COPY_MOVE(T)                                                \
    BL_DISABLE_COPY(T);                                                        \
    BL_DISABLE_MOVE(T)

struct non_copy_moveable
  : private non_copyable
  , private non_moveable
{
    constexpr non_copy_moveable() = default;
    constexpr ~non_copy_moveable() = default;
    BL_DISABLE_COPY_MOVE(non_copy_moveable);
};

} // namespace bl

#endif
