#ifndef FS_HPP
#define FS_HPP

#include "defs.h"

#include <string>
#include <vector>

namespace sys {

namespace fs {

struct MTime;

enum ObjectType {
    Any,
    File,
    Directory
};

bool cwd(const std::string& dir);

std::string dirname(const std::string& path);

std::string basename(const std::string& path);

MTime getMTime(const std::string& file);

std::string lookup(const std::vector<std::string>& dirs, const std::string& name);

bool exists(const std::string& path, ObjectType type = Any);

bool operator ==(const MTime& a, const MTime& b);
bool operator <(const MTime& a, const MTime& b);

inline bool operator !=(const MTime& a, const MTime& b) {  return !(a == b); }

inline bool operator <=(const MTime& a, const MTime& b) { return a < b || a == b; }

inline bool operator >=(const MTime& a, const MTime& b) { return !(a < b); }

inline bool operator >(const MTime& a, const MTime& b) { return !(a <= b); }

// default implementations
namespace def {

std::string lookup(const std::vector<std::string>& dirs, const std::string& name);

}

} // namespace fs

} // namespace sys

#ifdef SYSTEM_UNIX
#include "sys/fs/fs_unix.hpp"
#else
#error "no Filesystem implementation available"
#endif

#endif
