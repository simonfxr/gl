#ifndef SYS_FS_WINDOWS_HPP
#define SYS_FS_WINDOWS_HPP

#include "sys/conf.hpp"

namespace sys {

namespace fs {

const char SEPARATOR = '\\';
#define SIZEOF_FILETIME 8

// mimic FILETIME
struct ModificationTime {
    struct FILETIME {
        char data[SIZEOF_FILETIME];
    };
    FILETIME mtime;
    ModificationTime() {}
    ModificationTime(const FILETIME& _mtime) :
        mtime(_mtime) {}
};

} // namespace fs

} // namespace sys

#endif
