#ifndef DATA_BITSET_HPP
#define DATA_BITSET_HPP

#include "defs.h"
#include <vector>

struct BitSet {
    std::vector<bool> bits;

    BitSet(uint32 n = 0, bool val = false) :
        bits(n, val) {}

    bool operator [](uint32 i) const {
        return bits[i];
    }

    struct reference {
        BitSet& set;
        int i;
        reference(BitSet& s, uint32 idx) :
            set(s), i(idx) {}
        reference& operator =(bool val) {
            set.bits[i] = val;
            return *this;
        }
        operator bool() { return set.bits[i]; }
    };

    reference operator[](uint32 i) {
        return reference(*this, i);
    }
};

#endif
