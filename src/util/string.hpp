#ifndef UTIL_STRING_HPP
#define UTIL_STRING_HPP

#include "err/err.hpp"
#include "sys/io/Stream.hpp"

#include <string>
#include <string_view>

template<typename T>
constexpr size_t
string_length_at_least(const T &s)
{
    using S = std::remove_cv_t<T>;
    if constexpr (std::is_same_v<S, std::string> ||
                  std::is_same_v<S, std::string_view>) {
        return s.size();
    } else {
        return 0;
    }
}

template<typename... Args>
inline std::string
string_concat(Args &&...args)
{
    sys::io::ByteStream sstream;
    sstream.reserve((0 + ... + string_length_at_least(args)));
    (sstream << ... << std::forward<Args>(args));
    return bl::move(sstream).str();
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
