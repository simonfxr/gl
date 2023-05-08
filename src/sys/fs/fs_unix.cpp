#include "sys/fs/fs_unix.hpp"

#include "err/err.hpp"
#include "sys/fs.hpp"
#include "sys/strerror_unix.hpp"
#include "util/string.hpp"

#include <bl/core.hpp>
#include <bl/cstring.hpp>
#include <bl/string.hpp>
#include <bl/string_view.hpp>

#include <cerrno>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace sys::fs {

using namespace bl::literals;

bool
changeDirectory(bl::string_view dir)
{
    auto cstr = bl::cstring_view{ dir };
    if (chdir(cstr.c_str()) < 0) {
        ERR(strerror_errno(errno));
        return false;
    }
    return true;
}

bl::string
currentDirectory()
{
    using namespace bl::literals;
    bl::string path(96_z, '\0');

retry:
    for (;;) {
        while (!getcwd(path.data(), path.size())) {
            if (errno == ERANGE) {
                path.resize(path.size() * 2, '\0');
                goto retry;
            }
            if (errno != EINTR) {
                ERR(scat("getcwd() failed: "_sv, strerror_errno()));
                return ""_s;
            }
        }

        auto len = strlen(path.c_str());
        path.resize(len);
        return path;
    }
}

static bl::usize
dropTrailingSlashes(bl::string_view path)
{
    if (path.empty())
        return bl::string::npos;
    auto pos = path.size() - 1;

    while (pos > 0 && path[pos] == '/')
        --pos;

    return pos;
}

bl::string
dirname(bl::string_view path)
{
    if (path.empty())
        return "."_s;

    auto pos = path.rfind('/', dropTrailingSlashes(path));

    if (pos != bl::string::npos) {
        while (pos > 1 && path[pos - 1] == '/')
            --pos;
    } else {
        pos = 0;
    }

    if (pos == 0)
        return "/"_s;

    return path.first(pos);
}

bl::string
basename(bl::string_view path)
{

    if (path.empty())
        return ""_s;

    auto pos = path.rfind('/', dropTrailingSlashes(path));
    auto end = bl::string::npos;

    if (pos != bl::string::npos) {
        pos += 1;
        end = path.find('/', pos + 1);
    } else {
        pos = 0;
    }

    if (end == bl::string::npos)
        end = path.size();

    return path.subspan(pos, end - pos);
}

bl::string
dropExtension(bl::string_view path)
{
    return def::dropExtension(path);
}

bl::string
extension(bl::string_view path)
{
    auto pos = path.rfind('/');
    if (pos == bl::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == bl::string::npos)
        return ""_s;
    return path.subspan(pos + 1);
}

bool
isAbsolute(bl::string_view path)
{
    return !path.empty() && path[0] == '/';
}

bl::string
absolutePath(bl::string_view path)
{
    if (isAbsolute(path))
        return bl::string{ path };

    auto dir = currentDirectory();
    if (dir.empty())
        return ""_s;

    return join(dir, path);
}

bl::optional<Stat>
stat(bl::string_view path)
{
    Stat stat;
    stat.absolute = absolutePath(path);
    if (stat.absolute.empty())
        return bl::nullopt;
    if (auto mtime = modificationTime(stat.absolute)) {
        stat.mtime = *mtime;
        return stat;
    }
    return bl::nullopt;
}

bl::optional<FileTime>
modificationTime(bl::string_view path)
{
    struct stat st
    {};
    if (stat(bl::cstring_view(path).c_str(), &st) != 0) {
        auto err = errno;
        if (err != ENOENT)
            ERR(strerror_errno(err));
        return bl::nullopt;
    }
    return FileTime{ st.st_mtime };
}

bl::string
lookup(bl::span<const bl::string> dirs, bl::string_view path, ObjectType type)
{
    return def::lookup(dirs, path, type);
}

bl::optional<ObjectType>
exists(bl::string_view path)
{
    using stat = struct stat;
    stat info{};
    if (::stat(bl::cstring_view{ path }.c_str(), &info) < 0) {
        auto err = bl::exchange(errno, 0);
        if (err != ENOENT)
            ERR(strerror_errno(err));
        return bl::nullopt;
    }
    auto type =
      S_ISDIR(info.st_mode) ? ObjectType::Directory : ObjectType::File;
    return { type };
}

} // namespace sys::fs
