#ifndef DATA_BITSET_HPP
#define DATA_BITSET_HPP

#include "defs.hpp"
#include <vector>

struct BitSet
{
    std::vector<bool> bits;

    BitSet(defs::size_t n = 0, bool val = false) : bits(UNSIZE(n), val) {}

    bool operator[](defs::index_t i) const { return bits[UNSIZE(i)]; }

    struct reference
    {
        BitSet &set;
        defs::index_t i;

        reference(BitSet &s, defs::index_t idx) : set(s), i(ASSERT_SIZE(idx)) {}

        reference &operator=(bool val)
        {
            set.bits[defs::usize_t(i)] = val;
            return *this;
        }

        reference &operator=(const reference &ref)
        {
            set.bits[defs::usize_t(i)] = ref.set.bits[defs::usize_t(ref.i)];
            return *this;
        }

        operator bool() { return set.bits[defs::usize_t(i)]; }
    };

    reference operator[](defs::index_t i) { return reference(*this, i); }

    void set(bool val)
    {
        defs::size_t s = SIZE(bits.size());
        for (defs::index_t i = 0; i < s; ++i)
            bits[defs::usize_t(i)] = val;
    }

    void resize(defs::size_t s) { bits.resize(UNSIZE(s)); }
};

#endif
