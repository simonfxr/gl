#ifndef UTIL_STRING_UTILS_HPP
#define UTIL_STRING_UTILS_HPP

#include "err/err.hpp"

#include <sstream>
#include <string>
#include <string_view>

template<typename... Args>
std::string
string_concat(Args &&... args)
{
    std::stringstream sstream;
    (sstream << ... << std::forward<Args>(args));
    return sstream.str();
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
