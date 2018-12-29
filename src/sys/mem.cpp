#include "mem.hpp"

#include <cstring>

namespace sys {

namespace mem {

Ordering
compare(const char *a, size_t n, const char *b, size_t m)
{
    int cmp = memcmp(a, b, n < m ? n : m);
    if (cmp == 0 && n != m) {
        return n < m ? Ordering::LT : Ordering::GT;
    }
    return Ordering::EQ;
}

std::unique_ptr<char[]>
dup(const char *a, size_t n)
{
    std::unique_ptr<char[]> buf(n > 0 ? new char[n] : nullptr);
    if (n > 0)
        memcpy(buf.get(), a, n);
    return buf;
}

} // namespace mem

} // namespace sys
