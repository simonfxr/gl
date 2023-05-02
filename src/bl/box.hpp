#ifndef BL_BOX_HPP
#define BL_BOX_HPP

#include <bl/box.hpp>
#include <bl/core.hpp>
#include <bl/non_null.hpp>
#include <bl/owned.hpp>
#include <bl/raw_ptr.hpp>

namespace bl {

/**
 * Reference to a heap allocated object, the contained objects address is
 * guaranteed to be stable for its entire life time.
 *
 * The moved from state has given up its heap allocated object and only allows
 * calling the destructor.
 */
template<typename T, template<typename> class Rep = raw_ptr>
struct box : private owned<non_null<Rep<T>>>
{
    using Base = owned<non_null<Rep<T>>>;

    using element_type = T;
    static inline constexpr bool is_nullable = false;
    static inline constexpr bool is_owned = false;
    template<typename U>
    using rebind = box<U, Rep>;

    box() : Base(new_obj<T>()) {}
    box(const box &rhs) : Base(new_obj<T>(*rhs.get())) {}
    box(box &&rhs) noexcept = default;
    box &operator=(const box &rhs) { *Base::get() = *rhs.get(); }
    box &operator=(box &&rhs) noexcept { *Base::get() = bl::move(*rhs.get()); }
    ~box() = default;

    box(std::in_place_t, auto &&...args)
      : Base{ new_obj<T>(bl::forward<decltype(args)>(args)...) }
    {}

    using Base::operator*;
    using Base::operator->;
    using Base::get;

    friend bool operator==(const box<T, Rep> &, const box<T, Rep> &) = default;
};

} // namespace bl

#endif // BL_BOX_HPP
