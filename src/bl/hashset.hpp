#ifndef BL_HASHSET_HPP
#define BL_HASHSET_HPP

#include "bl/core.hpp"
#include "bl/equal.hpp"
#include "bl/hash.hpp"
#include "bl/pair.hpp"
#include "bl/vector.hpp"

namespace bl {

template<typename T, typename Hash = hash<T>, typename Equal = equal<T>>
struct hashset : private vector<T>
{

    using base_t = vector<T>;
    using iterator = decltype(declval<base_t &>().begin());
    using const_iterator = decltype(declval<const base_t &>().begin());

    using base_t::begin;
    using base_t::clear;
    using base_t::empty;
    using base_t::end;
    using base_t::size;

    template<typename U>
    iterator find(const U &arg)
    {
        Equal eq;
        auto end = base_t::end();
        for (auto it = base_t::begin(); it != end; ++it)
            if (eq(*it, arg))
                return it;
        return end;
    }

    template<typename U>
    bool contains(const U &arg)
    {
        auto it = find(arg);
        return it != base_t::end();
    }

    template<typename U>
    const_iterator find(const U &arg) const
    {
        return const_cast<hashset<T, Hash, Equal> &>(*this).find(arg);
    }

    template<typename U>
    pair<T &, bool> insert(U &&arg)
    {
        auto it = find(arg);
        if (it == base_t::end())
            return { base_t::emplace_back(std::forward<U>(arg)), true };
        return { *it, false };
    }

    template<typename U>
    pair<T &, bool> insert_or_assign(U &&arg)
    {
        auto it = find(arg);
        if (it == base_t::end())
            return { base_t::emplace_back(std::forward<U>(arg)), true };
        *it = arg;
        return { *it, false };
    }

    template<typename U>
    size_t erase(U &&arg)
    {
        auto it = find(arg);
        if (it == base_t::end())
            return 0;
        base_t::erase(it);
        return 1;
    }
};

} // namespace bl

#endif
