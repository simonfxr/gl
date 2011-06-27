#ifndef FS_HPP
#define FS_HPP

#include "defs.h"
#include <string>

namespace sys {

namespace fs {

struct MTime;

bool cwd(const std::string& dir);

std::string dirname(const std::string& path);

std::string basename(const std::string& path);

MTime getMTime(const std::string& file);

bool operator ==(const MTime& a, const MTime& b);
bool operator <(const MTime& a, const MTime& b);

inline bool operator !=(const MTime& a, const MTime& b) {  return !(a == b); }

inline bool operator <=(const MTime& a, const MTime& b) { return a < b || a == b; }

inline bool operator >=(const MTime& a, const MTime& b) { return !(a < b); }

inline bool operator >(const MTime& a, const MTime& b) { return !(a <= b); }

} // namespace fs

} // namespace sys

#ifdef SYSTEM_UNIX
#include "sys/fs/fs_unix.hpp"
#else
#error "no Filesystem implementation available"
#endif

#endif
