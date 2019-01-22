#include "sys/fs.hpp"

#include "err/err.hpp"
#include "sys/win_utf_conv.hpp"

#include <Shlwapi.h>
#include <Windows.h>

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
cwd(bl::string_view dir)
{
    auto dirstr = bl::string(dir);
    return SetCurrentDirectory(dirstr.c_str()) == TRUE;
}

bl::string
cwd()
{
    bl::wstring dir(MAX_PATH, '\0');
    auto len = GetCurrentDirectoryW(DWORD(dir.size()), dir.data());
    if (!len) {
        WARN("GetCurrentDirectory() failed");
        return "";
    }
    return absolutePath(utf16To8({ dir.data(), size_t(len) }));
}

bl::string
dirname(bl::string_view path)
{
    return def::dirname(path);
}

bl::string
basename(bl::string_view path)
{
    return def::basename(path);
}

bl::string
extension(bl::string_view path)
{
    return def::extension(path);
}

bl::string
dropExtension(bl::string_view path)
{
    return def::dropExtension(path);
}

bl::string
dropTrailingSeparators(bl::string_view path)
{
    return def::dropTrailingSeparators(path);
}

bool
isAbsolute(bl::string_view path)
{
    auto wstr = utf8To16(path);
    return PathIsRelativeW(wstr.c_str()) != TRUE;
}

bl::optional<FileTime>
modificationTime(bl::string_view path)
{
    return def::modificationTime(path);
}

bl::optional<Stat>
stat(bl::string_view path)
{
    auto wpath = utf8To16(path);
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    if (GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &attrs) == 0)
        return bl::nullopt;

    Stat stat;
    stat.mtime.seconds = filetimeToUnixTimestap(&attrs.ftLastWriteTime);
    stat.type = (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
                  ? ObjectType::Directory
                  : ObjectType::File;
    bl::wstring wabs(MAX_PATH, 0);
    auto len =
      GetFullPathNameW(wpath.c_str(), DWORD(wabs.size()), wabs.data(), nullptr);
    if (!len)
        return bl::nullopt;
    stat.absolute = utf16To8({ wabs.data(), len });
    return stat;
}

bl::string
absolutePath(bl::string_view path)
{
    auto wpath = utf8To16(path);
    bl::wstring abs(MAX_PATH, '\0');
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

bl::string
lookup(const bl::vector<bl::string> &dirs, bl::string_view path)
{
    return def::lookup(dirs, path);
}

bl::optional<ObjectType>
exists(bl::string_view path)
{
    return def::exists(path);
}

} // namespace fs

} // namespace sys
