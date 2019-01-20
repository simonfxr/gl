#ifndef UTIL_STRING_HPP
#define UTIL_STRING_HPP

#include "err/err.hpp"
#include "sys/io/Stream.hpp"

#include <string>
#include <string_view>

template<typename... Args>
inline std::string
string_concat(Args &&... args)
{
    sys::io::ByteStream sstream;
    (sstream << ... << std::forward<Args>(args));
    return std::move(sstream).str();
}

inline std::string
view_substr(std::string_view s, size_t begin)
{
    ASSERT(begin <= s.size());
    return { s.data() + begin, s.size() - begin };
}

inline std::string
view_substr(std::string_view s, size_t begin, size_t len)
{
    ASSERT(begin < s.size() && begin + len <= s.size());
    return { s.data() + begin, len };
}

#endif
