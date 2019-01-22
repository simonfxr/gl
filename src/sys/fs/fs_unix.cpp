#include "sys/fs/fs_unix.hpp"

#include "err/err.hpp"
#include "sys/fs.hpp"
#include "util/string.hpp"

#include "bl/string.hpp"
#include <cerrno>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace sys {
namespace fs {

bool
cwd(bl::string_view dir)
{
    if (chdir(bl::string(dir).c_str()) < 0) {
        ERR(strerror(bl::exchange(errno, 0)));
        return false;
    }
    return true;
}

bl::string
cwd()
{
    size_t size_t = 128;
    bl::string path;

retry:
    for (;;) {
        size_t *= 2;
        path.resize(size_t, '\0');
        while (!getcwd(path.data(), size_t)) {
            if (errno == ERANGE)
                goto retry;
            if (errno != EINTR) {
                ERR(string_concat("cwd() failed: ",
                                  strerror(bl::exchange(errno, 0))));
                return "";
            }
        }

        auto len = strlen(path.c_str());
        path.resize(len);
        return path;
    }
}

static size_t
dropTrailingSlashes(bl::string_view path)
{
    if (path.empty())
        return bl::string::npos;
    size_t pos = path.size() - 1;

    while (pos > 0 && path[pos] == '/')
        --pos;

    return pos;
}

bl::string
dirname(bl::string_view path)
{
    if (path.empty())
        return ".";

    size_t pos = path.rfind('/', dropTrailingSlashes(path));

    if (pos != bl::string::npos) {
        while (pos > 1 && path[pos - 1] == '/')
            --pos;
    } else {
        pos = 0;
    }

    if (pos == 0)
        return "/";

    return path.substr(0, pos);
}

bl::string
basename(bl::string_view path)
{

    if (path.empty())
        return "";

    size_t pos = path.rfind('/', dropTrailingSlashes(path));
    size_t end = bl::string::npos;

    if (pos != bl::string::npos) {
        pos += 1;
        end = path.find('/', pos + 1);
    } else {
        pos = 0;
    }

    if (end == bl::string::npos)
        end = path.size();

    return path.substr(pos, end - pos);
}

bl::string
dropExtension(bl::string_view path)
{
    return def::dropExtension(path);
}

bl::string
extension(bl::string_view path)
{
    size_t pos = path.rfind('/');
    if (pos == bl::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == bl::string::npos)
        return "";
    return path.substr(pos + 1);
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

    bl::string dir = cwd();
    if (dir.empty())
        return "";

    return string_concat(dir, "/", path);
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
    struct stat st;
    if (stat(bl::string(path).c_str(), &st) != 0) {
        auto err = bl::exchange(errno, 0);
        if (err != ENOENT)
            ERR(strerror(err));
        return bl::nullopt;
    }
    return FileTime{ st.st_mtime };
}

bl::string
lookup(bl::array_view<const bl::string> dirs, bl::string_view name)
{
    return def::lookup(dirs, name);
}

bl::optional<ObjectType>
exists(bl::string_view path)
{
    struct stat info;
    if (stat(bl::string(path).c_str(), &info) == -1) {
        auto err = bl::exchange(errno, 0);
        if (err != ENOENT)
            ERR(strerror(err));
        return bl::nullopt;
    }
    auto objtype = info.st_mode & S_IFMT;
    return { objtype == S_IFDIR ? ObjectType::Directory : ObjectType::File };
}

} // namespace fs
} // namespace sys
