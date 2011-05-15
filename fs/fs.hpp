#ifndef FS_HPP
#define FS_HPP

#include "defs.h"
#include <string>

#ifdef SYSTEM_UNIX
#include "fs/fs_unix.hpp"
#else
#error "no Filesystem implementation available"
#endif

namespace fs {

bool cwd(const std::string& dir);

bool cwdBasenameOf(const std::string& file);

MTime getMTime(const std::string& file);

bool operator ==(const MTime& a, const MTime& b);
bool operator <(const MTime& a, const MTime& b);

inline bool operator !=(const MTime& a, const MTime& b) {  return !(a == b); }

inline bool operator <=(const MTime& a, const MTime& b) { return a < b || a == b; }

inline bool operator >=(const MTime& a, const MTime& b) { return !(a < b); }

inline bool operator >(const MTime& a, const MTime& b) { return !(a <= b); }

} // namespace fs

#endif
