#ifndef UTIL_STRING_HPP
#define UTIL_STRING_HPP

#include "bl/string.hpp"
#include "err/err.hpp"
#include "sys/io/Stream.hpp"

template<typename... Args>
inline bl::string
string_concat(Args &&... args)
{
    sys::io::ByteStream sstream;
    (sstream << ... << std::forward<Args>(args));
    return std::move(sstream).str();
}

inline bl::string
string_concat(bl::string &&str) noexcept
{
    return std::move(str);
}

#endif
