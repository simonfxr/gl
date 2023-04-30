#ifndef SYS_IO_OPERATORS_HPP
#define SYS_IO_OPERATORS_HPP

#include "sys/conf.hpp"

#include <bl/tag_invoke.hpp>

namespace sys::io {

inline constexpr struct read_f
{
    BL_DEF_TAG_CPO_OPERATOR(read_f)
} read;

inline constexpr struct write_f
{
    BL_DEF_TAG_CPO_OPERATOR(write_f)
} write;

inline constexpr struct close_f
{
    BL_DEF_TAG_CPO_OPERATOR(close_f)
} close;

inline constexpr struct flush_f
{
    BL_DEF_TAG_CPO_OPERATOR(flush_f)
} flush;

} // namespace sys::io

#endif // SYS_IO_OPERATORS_HPP
