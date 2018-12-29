#ifndef DATA_BITSET_HPP
#define DATA_BITSET_HPP

#include "defs.hpp"
#include <vector>

struct BitSet
{
    std::vector<bool> bits;

    BitSet(defs::size n = 0, bool val = false) : bits(UNSIZE(n), val) {}

    bool operator[](defs::index i) const { return bits[UNSIZE(i)]; }

    struct reference
    {
        BitSet &set;
        defs::index i;

        reference(BitSet &s, defs::index idx) : set(s), i(ASSERT_SIZE(idx)) {}

        reference &operator=(bool val)
        {
            set.bits[defs::uptr(i)] = val;
            return *this;
        }

        reference &operator=(const reference &ref)
        {
            set.bits[defs::uptr(i)] = ref.set.bits[defs::uptr(ref.i)];
            return *this;
        }

        operator bool() { return set.bits[defs::uptr(i)]; }
    };

    reference operator[](defs::index i) { return reference(*this, i); }

    void set(bool val)
    {
        defs::size s = SIZE(bits.size());
        for (defs::index i = 0; i < s; ++i)
            bits[defs::uptr(i)] = val;
    }

    void resize(defs::size s) { bits.resize(UNSIZE(s)); }
};

#endif
