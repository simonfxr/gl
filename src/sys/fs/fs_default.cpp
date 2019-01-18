#include "sys/fs.hpp"

#include "data/string_utils.hpp"
#include "err/err.hpp"

#include <sstream>

namespace sys {
namespace fs {

PP_DEF_ENUM_IMPL(SYS_OBJECT_TYPE_ENUM_DEF)

namespace def {
namespace {

bool
is_pathsep(char c)
{
#ifdef HU_OS_WINDOWS
    return c == '\\' || c == '/';
#else
    return c == SEPARATOR;
#endif
}

size_t
dropTrailingSeps(std::string_view path)
{
    if (path.empty())
        return std::string::npos;
    size_t pos = path.length() - 1;

    while (pos > 0 && is_pathsep(path[pos]))
        --pos;

    return pos;
}
} // namespace

std::string
join(std::string_view path, const char **parts, size_t n)
{
    std::stringstream out;
    out << path;
    for (auto i = size_t{ 0 }; i < n; ++i) {
        out << SEPARATOR << parts[i];
    }
    return std::move(out).str();
}

std::string
dirname(std::string_view path)
{
    if (path.empty())
        return ".";

    auto len = path.size();
    while (len > 0 && !is_pathsep(path[len - 1]))
        --len;

    // FIXME: properly handling windows paths is going to be a pain...
    if (is_pathsep(path[len])) {
        while (len > 1 && is_pathsep(path[len - 1]))
            --len;
    } else {
        len = 0;
    }

    if (len == 0)
        return "/";

    return view_substr(path, 0, len);
}

std::string
basename(std::string_view path)
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

    return view_substr(path, pos, end - pos);
}

std::string
extension(std::string_view path)
{
    size_t pos = path.rfind(SEPARATOR);
    if (pos == std::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == std::string::npos)
        return "";
    return view_substr(path, pos + 1);
}

std::string
dropExtension(std::string_view path)
{
    size_t pos = path.rfind(SEPARATOR);
    if (pos == std::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == std::string::npos)
        return std::string(path);
    return view_substr(path, 0, pos);
}

std::string
dropTrailingSeparators(std::string_view path)
{
    size_t pos = dropTrailingSeps(path);
    if (pos == std::string::npos)
        return std::string(path);
    return view_substr(path, 0, pos);
}

std::optional<ObjectType>
exists(std::string_view path)
{
    if (auto st = stat(path))
        return st->type;
    return std::nullopt;
}

std::string
lookup(const std::vector<std::string> &dirs, std::string_view name)
{
    std::string suffix = string_concat("/", name);

    for (const auto &dir : dirs) {
        std::string filename = dir + suffix;
        if (sys::fs::exists(filename))
            return filename;
    }

    return "";
}

std::string
absolutePath(std::string_view path)
{
    if (auto st = stat(path))
        return st->absolute;
    return "";
}

std::optional<FileTime>
modificationTime(std::string_view path)
{
    if (auto st = stat(path))
        return st->mtime;
    return std::nullopt;
}

} // namespace def
} // namespace fs
} // namespace sys
