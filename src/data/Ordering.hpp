#ifndef DATA_ORDERING_HPP
#define DATA_ORDERING_HPP

#include "defs.hpp"

enum class Ordering : defs::int8_t
{
    LT = -1,
    EQ = 0,
    GT = 1
};

enum class Equality : defs::uint8_t
{
    EQ = 0,
    NEQ = 1
};

#endif
