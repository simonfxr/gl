#include "sys/fs.hpp"

#include "err/err.hpp"
#include "sys/io/Stream.hpp"
#include "util/string.hpp"

namespace sys::fs::def {

using namespace bl::literals;

namespace {

bool
is_pathsep(bl::string_byte c)
{
#ifdef HU_OS_WINDOWS
    return c == SEPARATOR_CHAR || c == UNIX_SEPARATOR_CHAR;
#else
    return c == SEPARATOR_CHAR;
#endif
}

bool
is_pathsep(bl::string_view s)
{
#ifdef HU_OS_WINDOWS
    return c == SEPARATOR || c == UNIX_SEPARATOR;
#else
    return s == SEPARATOR;
#endif
}

bl::usize
dropTrailingSeps(bl::string_view path)
{
    if (path.empty())
        return bl::string::npos;
    auto pos = path.size() - 1;

    while (pos > 0 && is_pathsep(path[pos]))
        --pos;

    return pos;
}
} // namespace

bl::string
join(bl::string_view path, bl::span<const bl::string_view> parts)
{
    sys::io::ByteStream out;
    out << path;
    for (auto subpath : parts)
        out << SEPARATOR << subpath;
    return bl::move(out).str();
}

bl::string
join(bl::string_view path, bl::string_view subpath)
{
    return scat(path, SEPARATOR, subpath);
}

bl::string
dirname(bl::string_view path)
{
    if (path.empty())
        return "."_s;

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
        return "/"_s;

    return view_substr(path, 0, len);
}

bl::string
basename(bl::string_view path)
{

    if (path.empty())
        return ""_s;

    auto pos = path.rfind(SEPARATOR_CHAR, dropTrailingSeps(path));
    auto end = bl::string::npos;

    if (pos != bl::string::npos) {
        pos += 1;
        end = path.find(SEPARATOR_CHAR, pos + 1);
    } else {
        pos = 0;
    }

    if (end == bl::string::npos)
        end = path.size();

    return view_substr(path, pos, end - pos);
}

bl::string
extension(bl::string_view path)
{
    auto pos = path.rfind(SEPARATOR_CHAR);
    if (pos == bl::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == bl::string::npos)
        return ""_s;
    return view_substr(path, pos + 1);
}

bl::string
dropExtension(bl::string_view path)
{
    auto pos = path.rfind(SEPARATOR_CHAR);
    if (pos == bl::string::npos)
        pos = 0;
    pos = path.find('.', pos);
    if (pos == bl::string::npos)
        return path;
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

bl::optional<ObjectType>
exists(bl::string_view path)
{
    if (auto st = stat(path))
        return st->type;
    return bl::nullopt;
}

bl::string
lookup(bl::span<const bl::string> dirs, bl::string_view name, ObjectType type)
{
    auto nm = bl::string{ name };
    for (const auto &dir : dirs) {
        auto filename = absolutePath(join(dir, name));
        if (sys::fs::exists(filename, type))
            return filename;
    }
    return ""_s;
}

bl::string
absolutePath(bl::string_view path)
{
    if (auto st = stat(path))
        return st->absolute;
    return ""_s;
}

bl::optional<FileTime>
modificationTime(bl::string_view path)
{
    if (auto st = stat(path))
        return st->mtime;
    return bl::nullopt;
}

} // namespace sys::fs::def
