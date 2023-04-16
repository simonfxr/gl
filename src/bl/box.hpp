#ifndef BL_BOX_HPP
#define BL_BOX_HPP

#include <bl/core.hpp>
#include <bl/non_null.hpp>
#include <bl/owned.hpp>
#include <bl/raw_ptr.hpp>

namespace bl {

template<typename T, template<typename> class Rep = raw_ptr>
struct box : private owned<non_null<Rep<T>>>
{
    using Base = owned<non_null<Rep<T>>>;

    using element_type = T;
    static inline constexpr bool is_nullable = false;
    static inline constexpr bool is_owned = false;
    template<typename U>
    using rebind = box<U, Rep>;

    box() : Base(new T) {}
    box(const box &rhs) : Base(new T(*rhs.get())) {}
    box(box &&rhs) noexcept = default;
    box &operator=(const box &rhs) { *Base::get() = *rhs.get(); }
    box &operator=(box &&rhs) noexcept = default;
    ~box() = default;

    box(std::in_place_t, auto &&...args)
        requires(sizeof...(args) > 0)
      : Base{ new T{ bl::forward<decltype(args)>(args)... } }
    {}

    friend bool operator==(const box<T, Rep> &, const box<T, Rep> &) = default;
};

} // namespace bl

#endif // BL_BOX_HPP
