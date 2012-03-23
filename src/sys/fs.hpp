#ifndef FS_HPP
#define FS_HPP

#include "sys/conf.hpp"

#include <string>
#include <vector>

#ifdef SYSTEM_UNIX
#include "sys/fs/fs_unix.hpp"
#elif defined(SYSTEM_WINDOWS)
#include "sys/fs/fs_windows.hpp"
#else
#error "no Filesystem implementation available"
#endif

namespace sys {

namespace fs {

using namespace defs;

struct ModificationTime;

extern SYS_API const ModificationTime MIN_MODIFICATION_TIME;

enum ObjectType {
    File,
    Directory
};

struct Stat {
    ObjectType type;
    std::string absolute;
    ModificationTime mtime;

    Stat() :
        type(), absolute(), mtime() {}
};

SYS_API bool cwd(const std::string& dir);

SYS_API std::string cwd();

SYS_API std::string dirname(const std::string& path);

SYS_API std::string basename(const std::string& path);

SYS_API std::string extension(const std::string& path);

SYS_API std::string dropExtension(const std::string& path);

SYS_API std::string dropTrailingSeparators(const std::string& path);

SYS_API bool isAbsolute(const std::string& path);

SYS_API bool modificationTime(const std::string& path, sys::fs::ModificationTime *mtime);

SYS_API bool stat(const std::string& path, sys::fs::Stat *stat);

SYS_API std::string absolutePath(const std::string&);

SYS_API std::string lookup(const std::vector<std::string>&, const std::string&);

SYS_API bool exists(const std::string& path, ObjectType *type);

SYS_API bool operator ==(const ModificationTime& a, const ModificationTime& b);
SYS_API bool operator <(const ModificationTime& a, const ModificationTime& b);

inline bool operator !=(const ModificationTime& a, const ModificationTime& b) {  return !(a == b); }
inline bool operator <=(const ModificationTime& a, const ModificationTime& b) { return a < b || a == b; }
inline bool operator >=(const ModificationTime& a, const ModificationTime& b) { return !(a < b); }
inline bool operator >(const ModificationTime& a, const ModificationTime& b) { return !(a <= b); }

// default implementations
namespace def {

SYS_API std::string dirname(const std::string& path);

SYS_API std::string basename(const std::string& path);

SYS_API std::string extension(const std::string& path);

SYS_API std::string dropExtension(const std::string& path);

SYS_API std::string dropTrailingSeparators(const std::string& path);

SYS_API bool exists(const std::string& path, ObjectType *type);

SYS_API std::string lookup(const std::vector<std::string>& dirs, const std::string& name);

SYS_API std::string absolutePath(const std::string&);

SYS_API bool modificationTime(const std::string& path, sys::fs::ModificationTime *);

}

} // namespace fs

} // namespace sys

#endif
