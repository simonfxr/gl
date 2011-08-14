#ifndef FS_UNIX_HPP
#define FS_UNIX_HPP

#include "defs.h"
#include <ctime>

namespace sys {

namespace fs {

const char SEPARATOR = '/';

struct ModificationTime {
    time_t timestamp;

    ModificationTime() : timestamp() {}
    ModificationTime(time_t ts) : timestamp(ts) {}
};

} // namespace fs

} // namespace sys

#endif
