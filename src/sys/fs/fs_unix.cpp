#include "sys/fs/fs_unix.hpp"

#include "err/err.hpp"
#include "data/string_utils.hpp"
#include "sys/fs.hpp"

#include <cerrno>
#include <cstring>
#include <ctime>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace sys {
namespace fs {

bool
cwd(std::string_view dir)
{
    if (chdir(dir.c_str()) < 0) {
        ERR(strerror(errno));
        return false;
    }
    return true;
}

std::string
cwd()
{
    size_t size_t = 128;
    std::string path;

retry:
    for (;;) {
        size_t *= 2;
        path.resize(size_t, '\0');
        while (!getcwd(path.data(), size_t)) {
            if (errno == ERANGE)
                goto retry;
            if (errno != EINTR) {
                ERR("cwd() failed");
                return "";
            }
        }

        auto len = strlen(path.c_str());
        path.resize(len);
        return path;
    }
}

static size_t
dropTrailingSlashes(std::string_view path)
{
    if (path.empty())
        return std::string::npos;
    size_t pos = path.length() - 1;

    while (pos > 0 && path[pos] == '/')
        --pos;

    return pos;
}

std::string
dirname(std::string_view path)
{
    if (path.empty())
        return ".";

    size_t pos = path.rfind('/', dropTrailingSlashes(path));

    if (pos != std::string::npos) {
        while (pos > 1 && path[pos - 1] == '/')
            --pos;
    } else {
        pos = 0;
    }

    if (pos == 0)
        return "/";

    return view_substr(path, 0, pos);
}

std::string
basename(std::string_view path)
{

    if (path.empty())
        return "";

    size_t pos = path.rfind('/', dropTrailingSlashes(path));
    size_t end = std::string::npos;

    if (pos != std::string::npos) {
        pos += 1;
        end = path.find('/', pos + 1);
    } else {
        pos = 0;
    }

    if (end == std::string::npos)
        end = path.length();

	return view_substr(path, pos, end - pos);
}

std::string
dropExtension(std::string_view path)
{
    return def::dropExtension(path);
}

std::string
extension(std::string_view path)
{
    size_t pos = path.rfind('/');
    if (pos == std::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == std::string::npos)
        return "";
    return view_substr(path, pos + 1);
}

bool
isAbsolute(std::string_view path)
{
    return !path.empty() && path[0] == '/';
}

std::string
absolutePath(std::string_view path)
{
    if (isAbsolute(path))
        return path;

    std::string dir = cwd();
    if (dir.empty())
        return "";

    return dir + "/" + path;
}

std::optional<Stat>
stat(std::string_view path)
{
    Stat stat;
    stat.absolute = absolutePath(path);
    if (stat.absolute.empty())
        return std::nullopt;
    if (auto mtime = modificationTime(stat.absolute)) {
        stat.mtime = *mtime;
        return stat;
    }
    return std::nullopt;
}

std::optional<FileTime>
modificationTime(std::string_view path)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        ERR(strerror(errno));
        return std::nullopt;
    }
    return FileTime{ st.st_mtime };
}

std::string
lookup(const std::vector<std::string> &dirs, std::string_view name)
{
    return def::lookup(dirs, name);
}

std::optional<ObjectType>
exists(std::string_view path)
{
    struct stat info;
    if (stat(path.c_str(), &info) == -1)
        return std::nullopt;
    auto objtype = info.st_mode & S_IFMT;
    return { objtype == S_IFDIR ? Directory : File };
}

} // namespace fs
} // namespace sys
