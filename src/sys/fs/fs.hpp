#ifndef FS_HPP
#define FS_HPP

#include "defs.hpp"

#include <string>
#include <vector>

#ifdef SYSTEM_UNIX
#include "sys/fs/fs_unix.hpp"
#elif SYSTEM_WINDOWS
#include "sys/fs/fs_windows.hpp"
#else
#error "no Filesystem implementation available"
#endif

namespace sys {

using namespace defs;

namespace fs {

struct ModificationTime;

extern const ModificationTime MIN_MODIFICATION_TIME;

enum ObjectType {
    File,
    Directory
};

struct Stat {
	ObjectType type;
    std::string absolute;
    ModificationTime mtime;
};

bool cwd(const std::string& dir);

std::string cwd();

std::string dirname(const std::string& path);

std::string basename(const std::string& path);

std::string extension(const std::string& path);

std::string dropExtension(const std::string& path);

std::string dropTrailingSeparators(const std::string& path);

bool isAbsolute(const std::string& path);

bool modificationTime(const std::string& path, sys::fs::ModificationTime *mtime);

bool stat(const std::string& path, sys::fs::Stat *stat);

std::string absolutePath(const std::string&);

std::string lookup(const std::vector<std::string>&, const std::string&);

bool exists(const std::string& path, ObjectType *type);

bool operator ==(const ModificationTime& a, const ModificationTime& b);
bool operator <(const ModificationTime& a, const ModificationTime& b);

inline bool operator !=(const ModificationTime& a, const ModificationTime& b) {  return !(a == b); }
inline bool operator <=(const ModificationTime& a, const ModificationTime& b) { return a < b || a == b; }
inline bool operator >=(const ModificationTime& a, const ModificationTime& b) { return !(a < b); }
inline bool operator >(const ModificationTime& a, const ModificationTime& b) { return !(a <= b); }

// default implementations
namespace def {

std::string dirname(const std::string& path);

std::string basename(const std::string& path);

std::string extension(const std::string& path);

std::string dropExtension(const std::string& path);

std::string dropTrailingSeparators(const std::string& path);

bool exists(const std::string& path, ObjectType *type);

std::string lookup(const std::vector<std::string>& dirs, const std::string& name);

std::string absolutePath(const std::string&);

bool modificationTime(const std::string& path, sys::fs::ModificationTime *);

}

} // namespace fs

} // namespace sys

#endif
