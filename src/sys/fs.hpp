#ifndef FS_HPP
#define FS_HPP

#include "sys/conf.hpp"

#include <bl/enum.hpp>
#include <pp/enum.hpp>

#if HU_OS_POSIX_P
#    include "sys/fs/fs_unix.hpp"
#elif HU_OS_WINDOWS_P
#    include "sys/fs/fs_windows.hpp"
#else
#    error "no Filesystem implementation available"
#endif

#include <bl/string.hpp>
#include <bl/string_view.hpp>
#include <optional>
#include <span>

namespace sys::fs {

struct FileTime
{
    int64_t seconds{};
};

inline constexpr FileTime MIN_FILE_TIME{};

PP_DEF_ENUM(ObjectType, uint8_t, File, Directory);

struct Stat
{
    ObjectType type{};
    bl::string absolute;
    FileTime mtime{};
};

HU_NODISCARD SYS_API bool
cwd(bl::string_view dir);

HU_NODISCARD SYS_API bl::string
cwd();

HU_NODISCARD SYS_API bl::string
dirname(bl::string_view path);

HU_NODISCARD SYS_API bl::string
basename(bl::string_view path);

HU_NODISCARD SYS_API bl::string
extension(bl::string_view path);

template<typename... Args>
HU_NODISCARD inline bl::string
join(bl::string_view path, Args &&...args);

HU_NODISCARD SYS_API bl::string
dropExtension(bl::string_view path);

HU_NODISCARD SYS_API bl::string
dropTrailingSeparators(bl::string_view path);

HU_NODISCARD SYS_API bool
isAbsolute(bl::string_view path);

HU_NODISCARD SYS_API std::optional<FileTime>
modificationTime(bl::string_view path);

SYS_API std::optional<Stat>
stat(bl::string_view path);

HU_NODISCARD SYS_API bl::string
absolutePath(bl::string_view path);

HU_NODISCARD SYS_API bl::string
lookup(std::span<const bl::string>, bl::string_view path);

HU_NODISCARD SYS_API std::optional<ObjectType>
exists(bl::string_view path);

HU_NODISCARD inline bool
exists(bl::string_view path, ObjectType ty)
{
    auto ret = exists(path);
    return ret && *ret == ty;
}

HU_NODISCARD inline bool
fileExists(bl::string_view path)
{
    return exists(path, ObjectType::File);
}

HU_NODISCARD inline bool
directoryExists(bl::string_view path)
{
    return exists(path, ObjectType::Directory);
}

HU_NODISCARD inline constexpr bool
operator==(const FileTime &a, const FileTime &b)
{
    return a.seconds == b.seconds;
}

HU_NODISCARD inline constexpr bool
operator<(const FileTime &a, const FileTime &b)
{
    return a.seconds < b.seconds;
}

HU_NODISCARD inline constexpr bool
operator!=(const FileTime &a, const FileTime &b)
{
    return !(a == b);
}

HU_NODISCARD inline constexpr bool
operator<=(const FileTime &a, const FileTime &b)
{
    return a < b || a == b;
}

HU_NODISCARD inline constexpr bool
operator>=(const FileTime &a, const FileTime &b)
{
    return !(a < b);
}

HU_NODISCARD inline constexpr bool
operator>(const FileTime &a, const FileTime &b)
{
    return !(a <= b);
}

// default implementations
namespace def {

HU_NODISCARD SYS_API bl::string
join(bl::string_view path, const char **, size_t n);

HU_NODISCARD SYS_API bl::string
dirname(bl::string_view path);

HU_NODISCARD SYS_API bl::string
basename(bl::string_view path);

HU_NODISCARD SYS_API bl::string
extension(bl::string_view path);

HU_NODISCARD SYS_API bl::string
dropExtension(bl::string_view path);

HU_NODISCARD SYS_API bl::string
dropTrailingSeparators(bl::string_view path);

HU_NODISCARD SYS_API std::optional<ObjectType>
exists(bl::string_view path);

HU_NODISCARD SYS_API bl::string
lookup(std::span<const bl::string> dirs, bl::string_view name);

HU_NODISCARD SYS_API bl::string absolutePath(bl::string_view);

HU_NODISCARD SYS_API std::optional<FileTime>
modificationTime(bl::string_view path);

} // namespace def

template<typename... Args>
HU_NODISCARD inline bl::string
join(bl::string_view path, Args &&...args)
{
    const char *parts[] = { std::forward<Args>(args)... };
    return def::join(path, parts, sizeof...(args));
}

} // namespace sys::fs

#endif
