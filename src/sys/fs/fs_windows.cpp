#include "sys/fs.hpp"

#include "err/err.hpp"
#include "sys/win_utf_conv.hpp"

#include <shlwapi.h>
#include <windows.h>

namespace sys {
namespace fs {

using sys::win::utf16To8;
using sys::win::utf8To16;

namespace {

#define WINDOWS_TICK 10000000
#define SEC_TO_UNIX_EPOCH 11644473600LL

int64_t
filetimeToUnixTimestap(const FILETIME *ft)
{
    auto ticks = (int64_t(ft->dwHighDateTime) << 32) | ft->dwLowDateTime;
    ticks = ticks / WINDOWS_TICK;
    return ticks - SEC_TO_UNIX_EPOCH;
}
} // namespace

bool
cwd(std::string_view dir)
{
    auto dirstr = std::string(dir);
    return SetCurrentDirectory(dirstr.c_str()) == TRUE;
}

std::string
cwd()
{
    std::wstring dir(MAX_PATH, '\0');
    auto len = GetCurrentDirectoryW(DWORD(dir.size()), dir.data());
    if (!len) {
        WARN("GetCurrentDirectory() failed");
        return "";
    }
    return absolutePath(utf16To8({ dir.data(), size_t(len) }));
}

std::string
dirname(std::string_view path)
{
    return def::dirname(path);
}

std::string
basename(std::string_view path)
{
    return def::basename(path);
}

std::string
extension(std::string_view path)
{
    return def::extension(path);
}

std::string
dropExtension(std::string_view path)
{
    return def::dropExtension(path);
}

std::string
dropTrailingSeparators(std::string_view path)
{
    return def::dropTrailingSeparators(path);
}

bool
isAbsolute(std::string_view path)
{
    auto wstr = utf8To16(path);
    return PathIsRelativeW(wstr.c_str()) != TRUE;
}

std::optional<FileTime>
modificationTime(std::string_view path)
{
    return def::modificationTime(path);
}

std::optional<Stat>
stat(std::string_view path)
{
    auto wpath = utf8To16(path);
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    if (GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &attrs) == 0)
        return std::nullopt;

    Stat stat;
    stat.mtime.seconds = filetimeToUnixTimestap(&attrs.ftLastWriteTime);
    stat.type = (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
                  ? ObjectType::Directory
                  : ObjectType::File;
    std::wstring wabs(MAX_PATH, 0);
    auto len =
      GetFullPathNameW(wpath.c_str(), DWORD(wabs.size()), wabs.data(), nullptr);
    if (!len)
        return std::nullopt;
    stat.absolute = utf16To8({ wabs.data(), len });
    return stat;
}

std::string
absolutePath(std::string_view path)
{
    auto wpath = utf8To16(path);
    std::wstring abs(MAX_PATH, '\0');
    auto len =
      GetFullPathNameW(wpath.c_str(), DWORD(abs.size()), abs.data(), NULL);
    if (!len)
        return "";

    if (len > abs.size()) {
        // FIXME: error on small buffer
        WARN("buffer too small");
        return "";
    }

    return utf16To8({ abs.data(), len });
}

std::string
lookup(ArrayView<const std::string> dirs, std::string_view name)
{
    return def::lookup(dirs, name);
}

std::optional<ObjectType>
exists(std::string_view path)
{
    return def::exists(path);
}

} // namespace fs

} // namespace sys
