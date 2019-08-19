#ifndef FS_HPP
#define FS_HPP

#include "bl/array_view.hpp"
#include "bl/optional.hpp"
#include "bl/string.hpp"
#include "bl/string_view.hpp"
#include "pp/enum.hpp"
#include "sys/conf.hpp"

#if HU_OS_POSIX_P
#    include "sys/fs/fs_unix.hpp"
#elif HU_OS_WINDOWS_P
#    include "sys/fs/fs_windows.hpp"
#else
#    error "no Filesystem implementation available"
#endif

namespace sys {
namespace fs {

struct FileTime
{
    int64_t seconds{};
};

inline constexpr FileTime MIN_FILE_TIME{};

#define SYS_OBJECT_TYPE_ENUM_DEF(T, V0, V)                                     \
    T(ObjectType, uint8_t, V0(File) V(Directory))

PP_DEF_ENUM_WITH_API(SYS_API, SYS_OBJECT_TYPE_ENUM_DEF);

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
join(bl::string_view path, Args &&... args);

HU_NODISCARD SYS_API bl::string
dropExtension(bl::string_view path);

HU_NODISCARD SYS_API bl::string
dropTrailingSeparators(bl::string_view path);

HU_NODISCARD SYS_API bool
isAbsolute(bl::string_view path);

HU_NODISCARD SYS_API bl::optional<FileTime>
modificationTime(bl::string_view path);

SYS_API bl::optional<Stat>
stat(bl::string_view path);

HU_NODISCARD SYS_API bl::string
absolutePath(bl::string_view path);

HU_NODISCARD SYS_API bl::string
lookup(bl::array_view<const bl::string>, bl::string_view path);

HU_NODISCARD SYS_API bl::optional<ObjectType>
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

HU_NODISCARD SYS_API bl::optional<ObjectType>
exists(bl::string_view path);

HU_NODISCARD SYS_API bl::string
lookup(bl::array_view<const bl::string> dirs, bl::string_view name);

HU_NODISCARD SYS_API bl::string absolutePath(bl::string_view);

HU_NODISCARD SYS_API bl::optional<FileTime>
modificationTime(bl::string_view path);

} // namespace def

template<typename... Args>
HU_NODISCARD inline bl::string
join(bl::string_view path, Args &&... args)
{
    const char *parts[] = { bl::forward<Args>(args)... };
    return def::join(path, parts, sizeof...(args));
}

} // namespace fs
} // namespace sys

#endif
