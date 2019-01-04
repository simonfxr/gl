#include <cstdio>
#include <sstream>

#define SYS_IO_STREAM_NOWARN 1

#include "err/err.hpp"
#include "sys/fs.hpp"

namespace sys {
namespace fs {
namespace def {

namespace {
size_t
dropTrailingSeps(const std::string &path)
{
    if (path.empty())
        return std::string::npos;
    size_t pos = path.length() - 1;

    while (pos > 0 && path[pos] == '/')
        --pos;

    return pos;
}
} // namespace

std::string
join(const std::string &path, const char **parts, size_t n)
{
    std::stringstream out;
    out << path;
    for (auto i = size_t{ 0 }; i < n; ++i) {
        out << SEPARATOR << parts[i];
    }
    return std::move(out).str();
}

std::string
dirname(const std::string &path)
{
    if (path.empty())
        return ".";

    size_t pos = path.rfind(SEPARATOR, dropTrailingSeps(path));

    if (pos != std::string::npos) {
        while (pos > 1 && path[pos - 1] == SEPARATOR)
            --pos;
    } else {
        pos = 0;
    }

    if (pos == 0)
        return "/";

    return path.substr(0, pos);
}

std::string
basename(const std::string &path)
{

    if (path.empty())
        return "";

    size_t pos = path.rfind(SEPARATOR, dropTrailingSeps(path));
    size_t end = std::string::npos;

    if (pos != std::string::npos) {
        pos += 1;
        end = path.find(SEPARATOR, pos + 1);
    } else {
        pos = 0;
    }

    if (end == std::string::npos)
        end = path.length();

    return path.substr(pos, end - pos);
}

std::string
extension(const std::string &path)
{
    size_t pos = path.rfind(SEPARATOR);
    if (pos == std::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == std::string::npos)
        return "";
    return path.substr(pos + 1);
}

std::string
dropExtension(const std::string &path)
{
    size_t pos = path.rfind(SEPARATOR);
    if (pos == std::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == std::string::npos)
        return path;
    return path.substr(0, pos);
}

std::string
dropTrailingSeparators(const std::string &path)
{
    size_t pos = dropTrailingSeps(path);
    if (pos == std::string::npos)
        return path;
    return path.substr(0, pos);
}

std::optional<ObjectType>
exists(const std::string &path)
{
    if (auto st = stat(path))
        return st->type;
    return std::nullopt;
}

std::string
lookup(const std::vector<std::string> &dirs, const std::string &name)
{
    std::string suffix = "/" + name;

    for (const auto &dir : dirs) {
        std::string filename = dir + suffix;
        FILE *file = fopen(filename.c_str(), "r");
        if (file != nullptr) {
            fclose(file);
            return filename;
        }
    }

    return "";
}

std::string
absolutePath(const std::string &path)
{
    if (auto st = stat(path))
        return st->absolute;
    return "";
}

std::optional<FileTime>
modificationTime(const std::string &path)
{
    if (auto st = stat(path))
        return st->mtime;
    return std::nullopt;
}

} // namespace def
} // namespace fs
} // namespace sys
