#ifndef FS_HPP
#define FS_HPP

#include "sys/conf.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#ifdef HU_OS_POSIX
#include "sys/fs/fs_unix.hpp"
#elif defined(HU_OS_WINDOWS)
#include "sys/fs/fs_windows.hpp"
#else
#error "no Filesystem implementation available"
#endif

namespace sys {
namespace fs {

// unix time stamp
struct FileTime
{
    int64_t seconds;
};

namespace {

inline constexpr FileTime MIN_FILE_TIME = {};

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
};

SYS_API HU_NODISCARD bool
cwd(std::string_view dir);

SYS_API HU_NODISCARD std::string
cwd();

SYS_API HU_NODISCARD std::string
dirname(std::string_view path);

SYS_API HU_NODISCARD std::string
basename(std::string_view path);

SYS_API HU_NODISCARD std::string
extension(std::string_view path);

template<typename... Args>
inline HU_NODISCARD std::string
join(std::string_view path, Args &&... args);

SYS_API HU_NODISCARD std::string
dropExtension(std::string_view path);

SYS_API HU_NODISCARD std::string
dropTrailingSeparators(std::string_view path);

SYS_API HU_NODISCARD bool
isAbsolute(std::string_view path);

SYS_API HU_NODISCARD std::optional<FileTime>
modificationTime(std::string_view path);

SYS_API std::optional<Stat>
stat(std::string_view path);

SYS_API HU_NODISCARD std::string
absolutePath(std::string_view path);

SYS_API HU_NODISCARD std::string
lookup(const std::vector<std::string> &, std::string_view path);

SYS_API HU_NODISCARD std::optional<ObjectType>
exists(std::string_view path);

HU_NODISCARD inline HU_NODISCARD bool
exists(std::string_view path, ObjectType ty)
{
    auto ret = exists(path);
    return ret && *ret == ty;
}

inline constexpr HU_NODISCARD bool
operator==(const FileTime &a, const FileTime &b)
{
    return a.seconds == b.seconds;
}

inline constexpr HU_NODISCARD bool
operator<(const FileTime &a, const FileTime &b)
{
    return a.seconds < b.seconds;
}

inline constexpr HU_NODISCARD bool
operator!=(const FileTime &a, const FileTime &b)
{
    return !(a == b);
}
inline constexpr HU_NODISCARD bool
operator<=(const FileTime &a, const FileTime &b)
{
    return a < b || a == b;
}
inline constexpr HU_NODISCARD bool
operator>=(const FileTime &a, const FileTime &b)
{
    return !(a < b);
}
inline constexpr HU_NODISCARD bool
operator>(const FileTime &a, const FileTime &b)
{
    return !(a <= b);
}

// default implementations
namespace def {

SYS_API HU_NODISCARD std::string
join(std::string_view path, const char **, size_t n);

SYS_API HU_NODISCARD std::string
dirname(std::string_view path);

SYS_API HU_NODISCARD std::string
basename(std::string_view path);

SYS_API HU_NODISCARD std::string
extension(std::string_view path);

SYS_API HU_NODISCARD std::string
dropExtension(std::string_view path);

SYS_API HU_NODISCARD std::string
dropTrailingSeparators(std::string_view path);

SYS_API HU_NODISCARD std::optional<ObjectType>
exists(std::string_view path);

SYS_API HU_NODISCARD std::string
lookup(const std::vector<std::string> &dirs, std::string_view name);

SYS_API HU_NODISCARD std::string absolutePath(std::string_view);

SYS_API HU_NODISCARD std::optional<FileTime>
modificationTime(std::string_view path);

} // namespace def

template<typename... Args>
inline std::string
join(std::string_view path, Args &&... args)
{
    const char *parts[] = { std::forward<Args>(args)... };
    return def::join(path, parts, sizeof...(args));
}

} // namespace fs
} // namespace sys

#endif
