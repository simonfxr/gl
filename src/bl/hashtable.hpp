#ifndef BL_HASHTABLE_HPP
#define BL_HASHTABLE_HPP

#include "bl/hashset.hpp"

namespace bl {

template<typename K, typename V, typename H = hash<K>, typename Eq = equal<K>>
struct MapEntry
{
    K key;
    V value;

    template<typename A, typename B>
    MapEntry(A &&x, B &&y) : key(bl::forward<A>(x)), value(bl::forward<B>(y))
    {}
};

template<typename K, typename V, typename H, typename Eq>
struct hash<MapEntry<K, V, H, Eq>>
{
    inline size_t operator()(const MapEntry<K, V, H, Eq> &x) const
    {
        return H{}(x.key);
    }

    template<typename U>
    inline size_t operator()(const U &x) const
    {
        return H{}(x);
    }
};

template<typename K, typename V, typename H, typename Eq>
struct equal<MapEntry<K, V, H, Eq>>
{
    template<typename U>
    inline size_t operator()(const MapEntry<K, V, H, Eq> &lhs,
                             const U &rhs) const
    {
        return Eq{}(lhs.key, rhs);
    }

    inline size_t operator()(const MapEntry<K, V, H, Eq> &lhs,
                             const MapEntry<K, V, H, Eq> &rhs) const
    {
        return Eq{}(lhs.key, rhs.key);
    }
};

template<typename K, typename V, typename H = hash<K>, typename Eq = equal<K>>
struct hashtable : private hashset<MapEntry<K, V, H, Eq>>
{
    using entry_t = MapEntry<K, V, H, Eq>;
    using base_t = hashset<entry_t>;

    using base_t::begin;
    using base_t::clear;
    using base_t::contains;
    using base_t::empty;
    using base_t::end;
    using base_t::erase;
    using base_t::find;
    using base_t::size;

    template<typename KT, typename VT>
    pair<entry_t &, bool> insert(KT &&k, VT &&v)
    {
        auto it = base_t::find(k);
        if (it == base_t::end())
            return base_t::insert(
              entry_t(bl::forward<KT>(k), bl::forward<VT>(v)));
        return { *it, false };
    }

    template<typename KT, typename VT>
    pair<entry_t &, bool> insert_or_assign(KT &&k, VT &&v)
    {
        return base_t::insert_or_assign(
          entry_t(bl::forward<KT>(k), bl::forward<VT>(v)));
    }

    template<typename KT>
    V &operator[](KT &&k)
    {
        auto it = base_t::find(k);
        if (it != base_t::end())
            return it->value;
        return insert(bl::forward<KT>(k), V()).fst().value;
    }
};

} // namespace bl

#endif
