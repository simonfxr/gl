#include "err/err.hpp"
#include "sys/fs.hpp"

#include <Shlwapi.h>
#include <windows.h>

namespace sys {
namespace fs {

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

std::wstring
utf8ToUtf16(const std::string &str)
{
    if (str.empty())
        return std::wstring{};
    auto len =
      MultiByteToWideChar(CP_UTF8, 0, str.data(), int(str.size()), nullptr, 0);
    if (!len)
        FATAL_ERR("MultiByteToWideChar failed (size)");
    std::wstring ret(size_t(len), wchar_t{});
    len = MultiByteToWideChar(
      CP_UTF8, 0, str.data(), int(str.size()), ret.data(), len);
    if (!len)
        FATAL_ERR("MultiByteToWideChar failed");
    return ret;
}

std::string
utf16ToUtf8(const wchar_t *data, size_t sz)
{
    if (!sz)
        return std::string{};
    auto len = WideCharToMultiByte(
      CP_UTF8, 0, data, int(sz), nullptr, 0, nullptr, nullptr);
    if (!len)
        FATAL_ERR("WideCharToMultiByte failed (size)");
    std::string ret(size_t(len), wchar_t{});
    len = WideCharToMultiByte(
      CP_UTF8, 0, data, int(sz), ret.data(), len, nullptr, nullptr);
    if (!len)
        FATAL_ERR("WideCharToMultiByte failed");
    ret.resize(strlen(ret.c_str()));
    return ret;
}

} // namespace

bool
cwd(const std::string &dir)
{
    return SetCurrentDirectory(dir.c_str()) == TRUE;
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
    return absolutePath(utf16ToUtf8(dir.data(), len));
}

std::string
dirname(const std::string &path)
{
    return def::dirname(path);
}

std::string
basename(const std::string &path)
{
    return def::basename(path);
}

std::string
extension(const std::string &path)
{
    return def::extension(path);
}

std::string
dropExtension(const std::string &path)
{
    return def::dropExtension(path);
}

std::string
dropTrailingSeparators(const std::string &path)
{
    return def::dropTrailingSeparators(path);
}

bool
isAbsolute(const std::string &path)
{
    auto wstr = utf8ToUtf16(path);
    return PathIsRelativeW(wstr.c_str()) != TRUE;
}

std::optional<FileTime>
modificationTime(const std::string &path)
{
    return def::modificationTime(path);
}

std::optional<Stat>
stat(const std::string &path)
{
    auto wpath = utf8ToUtf16(path);
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    if (GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &attrs) == 0)
        return std::nullopt;

    Stat stat;
    stat.mtime.seconds = filetimeToUnixTimestap(&attrs.ftLastWriteTime);
    stat.type = (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
                  ? Directory
                  : File;
    std::wstring wabs(MAX_PATH, 0);
    auto len =
      GetFullPathNameW(wpath.c_str(), DWORD(wabs.size()), wabs.data(), nullptr);
    if (!len)
        return std::nullopt;
    stat.absolute = utf16ToUtf8(wabs.data(), len);
    return stat;
}

std::string
absolutePath(const std::string &path)
{
    auto wpath = utf8ToUtf16(path);
    std::wstring abs(MAX_PATH, '\0');
    auto len = GetFullPathNameW(wpath.c_str(), DWORD(abs.size()), abs.data(), NULL);
    if (!len)
        return "";

    if (len > abs.size()) {
        // FIXME: error on small buffer
        WARN("buffer too small");
        return "";
    }

    return utf16ToUtf8(abs.data(), len);
}

std::string
lookup(const std::vector<std::string> &dirs, const std::string &path)
{
    return def::lookup(dirs, path);
}

std::optional<ObjectType>
exists(const std::string &path)
{
    return def::exists(path);
}

} // namespace fs

} // namespace sys
