#include "sys/fs.hpp"

#include "err/err.hpp"
#include "sys/io/Stream.hpp"
#include "util/string.hpp"

namespace sys::fs {

using namespace std::literals;

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
dropTrailingSeps(bl::string_view path)
{
    if (path.empty())
        return bl::string::npos;
    size_t pos = path.length() - 1;

    while (pos > 0 && is_pathsep(path[pos]))
        --pos;

    return pos;
}
} // namespace

bl::string
join(bl::string_view path, const char **parts, size_t n)
{
    sys::io::ByteStream out;
    out << path;
    for (auto i = size_t{ 0 }; i < n; ++i)
        out << SEPARATOR << parts[i];
    return bl::move(out).str();
}

bl::string
dirname(bl::string_view path)
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

bl::string
basename(bl::string_view path)
{

    if (path.empty())
        return "";

    size_t pos = path.rfind(SEPARATOR, dropTrailingSeps(path));
    size_t end = bl::string::npos;

    if (pos != bl::string::npos) {
        pos += 1;
        end = path.find(SEPARATOR, pos + 1);
    } else {
        pos = 0;
    }

    if (end == bl::string::npos)
        end = path.length();

    return view_substr(path, pos, end - pos);
}

bl::string
extension(bl::string_view path)
{
    size_t pos = path.rfind(SEPARATOR);
    if (pos == bl::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == bl::string::npos)
        return "";
    return view_substr(path, pos + 1);
}

bl::string
dropExtension(bl::string_view path)
{
    size_t pos = path.rfind(SEPARATOR);
    if (pos == bl::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == bl::string::npos)
        return bl::string(path);
    return view_substr(path, 0, pos);
}

bl::string
dropTrailingSeparators(bl::string_view path)
{
    auto pos = dropTrailingSeps(path);
    if (pos == bl::string::npos)
        return bl::string(path);
    return view_substr(path, 0, pos);
}

std::optional<ObjectType>
exists(bl::string_view path)
{
    if (auto st = stat(path))
        return st->type;
    return std::nullopt;
}

bl::string
lookup(std::span<const bl::string> dirs, bl::string_view name)
{
    auto suffix = string_concat("/", name);

    for (const auto &dir : dirs)
        if (auto filename = dir + suffix; sys::fs::exists(filename))
            return filename;

    return ""s;
}

bl::string
absolutePath(bl::string_view path)
{
    if (auto st = stat(path))
        return st->absolute;
    return "";
}

std::optional<FileTime>
modificationTime(bl::string_view path)
{
    if (auto st = stat(path))
        return st->mtime;
    return std::nullopt;
}

} // namespace def
} // namespace sys::fs
