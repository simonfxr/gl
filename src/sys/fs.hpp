#ifndef FS_HPP
#define FS_HPP

#include "sys/conf.hpp"

#include <string>
#include <vector>

#ifdef HU_OS_POSIX_P
#include "sys/fs/fs_unix.hpp"
#elif defined(HU_OS_WINDOWS_P)
#include "sys/fs/fs_windows.hpp"
#else
#error "no Filesystem implementation available"
#endif

namespace sys {
namespace fs {

// unix time stamp
struct FileTime
{
    uint32_t seconds;
};

namespace {

const ATTRS(ATTR_NO_WARN_UNUSED_DEF) FileTime MIN_FILE_TIME = {};

} // namespace

enum ObjectType
{
    File,
    Directory
};

struct Stat
{
    ObjectType type;
    std::string absolute;
    FileTime mtime;

    Stat() : type(), absolute(), mtime() {}
};

SYS_API bool
cwd(const std::string &dir);

SYS_API std::string
cwd();

SYS_API std::string
dirname(const std::string &path);

SYS_API std::string
basename(const std::string &path);

SYS_API std::string
extension(const std::string &path);

template<typename... Args>
inline std::string
join(const std::string &path, Args &&... args);

SYS_API std::string
dropExtension(const std::string &path);

SYS_API std::string
dropTrailingSeparators(const std::string &path);

SYS_API bool
isAbsolute(const std::string &path);

SYS_API bool
modificationTime(const std::string &path, sys::fs::FileTime *mtime);

SYS_API bool
stat(const std::string &path, sys::fs::Stat *stat);

SYS_API std::string
absolutePath(const std::string &);

SYS_API std::string
lookup(const std::vector<std::string> &, const std::string &);

SYS_API bool
exists(const std::string &path, ObjectType *type);

inline bool
operator==(const FileTime &a, const FileTime &b)
{
    return a.seconds == b.seconds;
}

inline bool
operator<(const FileTime &a, const FileTime &b)
{
    return a.seconds < b.seconds;
}

inline bool
operator!=(const FileTime &a, const FileTime &b)
{
    return !(a == b);
}
inline bool
operator<=(const FileTime &a, const FileTime &b)
{
    return a < b || a == b;
}
inline bool
operator>=(const FileTime &a, const FileTime &b)
{
    return !(a < b);
}
inline bool
operator>(const FileTime &a, const FileTime &b)
{
    return !(a <= b);
}

// default implementations
namespace def {

SYS_API std::string
join(const std::string &path, const char **, size_t n);

SYS_API std::string
dirname(const std::string &path);

SYS_API std::string
basename(const std::string &path);

SYS_API std::string
extension(const std::string &path);

SYS_API std::string
dropExtension(const std::string &path);

SYS_API std::string
dropTrailingSeparators(const std::string &path);

SYS_API bool
exists(const std::string &path, ObjectType *type);

SYS_API std::string
lookup(const std::vector<std::string> &dirs, const std::string &name);

SYS_API std::string
absolutePath(const std::string &);

SYS_API bool
modificationTime(const std::string &path, sys::fs::FileTime *);

} // namespace def

template<typename... Args>
inline std::string
join(const std::string &path, Args &&... args)
{
    const char *parts[] = { std::forward<Args>(args)... };
    return def::join(path, parts, sizeof...(args));
}

} // namespace fs
} // namespace sys

#endif
