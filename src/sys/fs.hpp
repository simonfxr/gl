#ifndef FS_HPP
#define FS_HPP

#include "pp/enum.hpp"
#include "sys/conf.hpp"
#include "util/ArrayView.hpp"

#if HU_OS_POSIX_P
#    include "sys/fs/fs_unix.hpp"
#elif HU_OS_WINDOWS_P
#    include "sys/fs/fs_windows.hpp"
#else
#    error "no Filesystem implementation available"
#endif

#include <optional>
#include <string>
#include <string_view>

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
    std::string absolute;
    FileTime mtime{};
};

HU_NODISCARD SYS_API bool
cwd(std::string_view dir);

HU_NODISCARD SYS_API std::string
cwd();

HU_NODISCARD SYS_API std::string
dirname(std::string_view path);

HU_NODISCARD SYS_API std::string
basename(std::string_view path);

HU_NODISCARD SYS_API std::string
extension(std::string_view path);

template<typename... Args>
HU_NODISCARD inline std::string
join(std::string_view path, Args &&...args);

HU_NODISCARD SYS_API std::string
dropExtension(std::string_view path);

HU_NODISCARD SYS_API std::string
dropTrailingSeparators(std::string_view path);

HU_NODISCARD SYS_API bool
isAbsolute(std::string_view path);

HU_NODISCARD SYS_API std::optional<FileTime>
modificationTime(std::string_view path);

SYS_API std::optional<Stat>
stat(std::string_view path);

HU_NODISCARD SYS_API std::string
absolutePath(std::string_view path);

HU_NODISCARD SYS_API std::string
lookup(ArrayView<const std::string>, std::string_view path);

HU_NODISCARD SYS_API std::optional<ObjectType>
exists(std::string_view path);

HU_NODISCARD inline bool
exists(std::string_view path, ObjectType ty)
{
    auto ret = exists(path);
    return ret && *ret == ty;
}

HU_NODISCARD inline bool
fileExists(std::string_view path)
{
    return exists(path, ObjectType::File);
}

HU_NODISCARD inline bool
directoryExists(std::string_view path)
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

HU_NODISCARD SYS_API std::string
join(std::string_view path, const char **, size_t n);

HU_NODISCARD SYS_API std::string
dirname(std::string_view path);

HU_NODISCARD SYS_API std::string
basename(std::string_view path);

HU_NODISCARD SYS_API std::string
extension(std::string_view path);

HU_NODISCARD SYS_API std::string
dropExtension(std::string_view path);

HU_NODISCARD SYS_API std::string
dropTrailingSeparators(std::string_view path);

HU_NODISCARD SYS_API std::optional<ObjectType>
exists(std::string_view path);

HU_NODISCARD SYS_API std::string
lookup(ArrayView<const std::string> dirs, std::string_view name);

HU_NODISCARD SYS_API std::string absolutePath(std::string_view);

HU_NODISCARD SYS_API std::optional<FileTime>
modificationTime(std::string_view path);

} // namespace def

template<typename... Args>
HU_NODISCARD inline std::string
join(std::string_view path, Args &&...args)
{
    const char *parts[] = { std::forward<Args>(args)... };
    return def::join(path, parts, sizeof...(args));
}

} // namespace fs
} // namespace sys

#endif
