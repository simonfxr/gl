#ifndef FS_UNIX_HPP
#define FS_UNIX_HPP

#include "defs.h"
#include <ctime>

namespace fs {

struct MTime {
    time_t timestamp;
};

} // namespace fs

#endif
