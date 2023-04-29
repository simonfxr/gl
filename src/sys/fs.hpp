#ifndef FS_HPP
#define FS_HPP

#include "sys/conf.hpp"

#include <bl/enum.hpp>
#include <bl/optional.hpp>
#include <bl/span.hpp>
#include <bl/string.hpp>
#include <bl/string_view.hpp>
#include <pp/enum.hpp>

#if HU_OS_POSIX_P
#    include "sys/fs/fs_unix.hpp"
#elif HU_OS_WINDOWS_P
#    include "sys/fs/fs_windows.hpp"
#else
#    error "no Filesystem implementation available"
#endif

namespace sys::fs {

using namespace bl::literals;

const bl::string_byte UNIX_SEPARATOR_CHAR = '/';
constexpr inline bl::string_view UNIX_SEPARATOR = "/"_sv;

struct FileTime
{
    int64_t seconds{};

    friend constexpr bool operator==(FileTime, FileTime) noexcept = default;
    friend constexpr auto operator<=>(FileTime, FileTime) noexcept = default;
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
lookup(bl::span<const bl::string>, bl::string_view path, ObjectType type);

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

// default implementations
namespace def {

HU_NODISCARD SYS_API bl::string
join(bl::string_view path, bl::span<const bl::string_view>);

HU_NODISCARD SYS_API bl::string
join(bl::string_view path, bl::string_view subpath);

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
lookup(bl::span<const bl::string> dirs, bl::string_view name, ObjectType type);

HU_NODISCARD SYS_API bl::string absolutePath(bl::string_view);

HU_NODISCARD SYS_API bl::optional<FileTime>
modificationTime(bl::string_view path);

} // namespace def

template<typename... Args>
HU_NODISCARD inline bl::string
join(bl::string_view path, Args &&...args)
    requires(std::same_as<std::decay_t<Args>, bl::string_view> && ...)
{
    auto paths =
      std::array{ static_cast<bl::string_view>(std::forward<Args>(args))... };
    return def::join(path, paths);
}

template<typename... Args>
HU_NODISCARD inline bl::string
join(bl::string_view path, bl::string_view subpath)
{
    return def::join(path, subpath);
}

} // namespace sys::fs

#endif
