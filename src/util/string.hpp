#ifndef UTIL_STRING_HPP
#define UTIL_STRING_HPP

#include "bl/string.hpp"
#include "err/err.hpp"
#include "sys/io/Stream.hpp"

#if HU_COMP_INTEL_P

namespace detail {
inline void
string_concat_aux(sys::io::ByteStream &out)
{}

template<typename Arg, typename... Args>
inline void
string_concat_aux(sys::io::ByteStream &out, Arg &&arg, Args &&... args)
{
    out << std::forward<Arg>(arg);
    string_concat_aux(out, std::forward<Args>(args)...);
}

} // namespace detail

template<typename... Args>
inline bl::string
string_concat(Args &&... args)
{
    sys::io::ByteStream sstream;
    detail::string_concat_aux(sstream, std::forward<Args>(args)...);
    return std::move(sstream).str();
}

#else
template<typename... Args>
inline bl::string
string_concat(Args &&... args)
{
    sys::io::ByteStream sstream;
    (sstream << ... << std::forward<Args>(args));
    return std::move(sstream).str();
}
#endif

inline bl::string
string_concat(bl::string &&str) noexcept
{
    return std::move(str);
}

#endif
