#ifndef FS_UNIX_HPP
#define FS_UNIX_HPP

#include "defs.h"
#include <ctime>

namespace sys {

namespace fs {

struct MTime {
    time_t timestamp;
};

} // namespace fs

} // namespace sys


#endif
