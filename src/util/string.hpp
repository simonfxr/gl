#ifndef UTIL_STRING_HPP
#define UTIL_STRING_HPP

#include "err/err.hpp"

#include "sys/io/Stream.hpp"

#include <bl/string.hpp>
#include <bl/string_view.hpp>

template<typename T>
constexpr size_t
string_length_at_least(const T &s)
{
    using S = std::remove_cv_t<T>;
    if constexpr (std::is_same_v<S, bl::string> ||
                  std::is_same_v<S, bl::string_view>) {
        return s.size();
    } else {
        return 0;
    }
}

template<typename... Args>
inline bl::string
scat(Args &&...args)
{
    sys::io::ByteStream sstream;
    sstream.reserve((0 + ... + string_length_at_least(args)));
    (sstream << ... << bl_fwd(args));
    return bl::move(sstream).str();
}

inline bl::string_view
view_substr(bl::string_view s, size_t begin)
{
    ASSERT(begin <= s.size());
    return { s.data() + begin, s.size() - begin };
}

inline bl::string_view
view_substr(bl::string_view s, size_t begin, size_t len)
{
    ASSERT(begin <= s.size() && begin + len <= s.size());
    return { s.data() + begin, len };
}

#endif
