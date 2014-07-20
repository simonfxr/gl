#ifndef SYS_MEM_HPP
#define SYS_MEM_HPP

#include "defs.hpp"
#include "data/Ordering.hpp"

#include <memory>

namespace sys {

namespace mem {

Ordering compare(const char *, size_t, const char *, size_t);

std::unique_ptr<char[]> dup(const char *, size_t);

} // namespace mem

} // namespace sys


#endif
