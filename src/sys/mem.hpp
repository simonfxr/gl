#ifndef SYS_MEM_HPP
#define SYS_MEM_HPP

#include "data/Ordering.hpp"

#include <memory>

namespace sys {

namespace mem {

Ordering
compare(const char *, defs::size_t, const char *, defs::size_t);

std::unique_ptr<char[]>
dup(const char *, defs::size_t);

} // namespace mem

} // namespace sys

#endif
