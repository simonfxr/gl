#ifndef BL_VEC_HPP
#define BL_VEC_HPP

#include <bl/span.hpp>

#include <vector>

namespace bl {

template<class T, typename Allocator = std::allocator<T>>
struct vec : private std::vector<T, Allocator>
{
    using Base = std::vector<T, Allocator>;

    using typename Base::allocator_type;
    using typename Base::const_iterator;
    using typename Base::const_pointer;
    using typename Base::const_reference;
    using typename Base::const_reverse_iterator;
    using typename Base::difference_type;
    using typename Base::iterator;
    using typename Base::pointer;
    using typename Base::reference;
    using typename Base::reverse_iterator;
    using typename Base::size_type;
    using typename Base::value_type;

    using std::vector<T, Allocator>::vector;
    using Base::back;
    using Base::begin;
    using Base::clear;
    using Base::emplace_back;
    using Base::empty;
    using Base::end;
    using Base::erase;
    using Base::insert;
    using Base::pop_back;
    using Base::push_back;
    using Base::rbegin;
    using Base::rend;
    using Base::reserve;
    using Base::resize;
    using Base::size;
    using Base::operator[];

    template<typename Elem, size_t N>
    vec(Elem (&)[N]) = delete;

    constexpr vec(const span<T> &rhs) : vec(rhs.begin(), rhs.end()) {}

    /* implicit */ constexpr operator span<T>() noexcept
    {
        return { data(), size() };
    }

    friend constexpr bool operator==(const vec<T, Allocator> &lhs,
                                     const vec<T, Allocator> &rhs) noexcept
    {
        return static_cast<span<const T>>(lhs) ==
               static_cast<span<const T>>(rhs);
    }

    friend constexpr bool operator==(const vec<T, Allocator> &lhs,
                                     const span<const T> &rhs) noexcept
    {
        return static_cast<span<T>>(lhs) == rhs;
    }

    friend constexpr bool operator==(const span<T> &lhs,
                                     const vec<T, Allocator> &rhs) noexcept
    {
        return lhs == static_cast<const span<T>>(rhs);
    }

    constexpr decltype(auto) data() const noexcept { return Base::data(); }

    constexpr decltype(auto) data() noexcept { return Base::data(); }
};

} // namespace bl

#endif // BL_VEC_HPP
