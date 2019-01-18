#ifndef DATA_BITSET_HPP
#define DATA_BITSET_HPP

#include "defs.hpp"
#include <vector>

struct BitSet
{
    std::vector<bool> bits;

    BitSet(size_t n = 0, bool val = false) : bits(n, val) {}

    bool operator[](size_t i) const { return bits[i]; }

    struct reference
    {
        BitSet &set;
        size_t i;

        reference(BitSet &s, size_t idx) : set(s), i(idx) {}

        reference &operator=(bool val)
        {
            set.bits[i] = val;
            return *this;
        }

        reference &operator=(const reference &ref)
        {
            set.bits[i] = ref.set.bits[ref.i];
            return *this;
        }

        operator bool() { return set.bits[i]; }
    };

    reference operator[](size_t i) { return reference(*this, i); }

    void set(bool val)
    {
        size_t s = bits.size();
        for (size_t i = 0; i < s; ++i)
            bits[i] = val;
    }

    void resize(size_t s) { bits.resize(s); }
};

#endif
