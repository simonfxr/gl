#ifndef SYS_IO_STREAM_FWD_HPP
#define SYS_IO_STREAM_FWD_HPP

#include "bl/string_fwd.hpp"
#include "bl/string_view_fwd.hpp"
#include "pp/enum.hpp"
#include "sys/conf.hpp"

#undef EOF
#undef stdin
#undef stdout
#undef stderr

namespace sys {
namespace io {

struct SYS_API OutStream;
struct SYS_API InStream;
struct SYS_API StreamEndl;
struct SYS_API IOStream;

HU_NODISCARD SYS_API InStream &
stdin();

HU_NODISCARD SYS_API OutStream &
stdout();

HU_NODISCARD SYS_API OutStream &
stderr();

extern SYS_API const StreamEndl endl;

#define SYS_STREAM_RESULT_ENUM_DEF(T, V0, V)                                   \
    T(StreamResult, uint8_t, V0(OK) V(Blocked) V(EOF) V(Closed) V(Error))

PP_FWD_DEF_ENUM_WITH_API(SYS_API, SYS_STREAM_RESULT_ENUM_DEF);

using StreamFlags = uint8_t;

inline constexpr StreamFlags SF_IN_EOF = 1;
inline constexpr StreamFlags SF_OUT_EOF = 2;
inline constexpr StreamFlags SF_IN_CLOSED = 4;
inline constexpr StreamFlags SF_OUT_CLOSED = 8;
inline constexpr StreamFlags SF_CLOSABLE = 16;

struct SYS_API InStream;
struct SYS_API OutStream;
struct SYS_API IOStream;
struct SYS_API NullStream;
struct SYS_API ByteStream;
struct SYS_API CooperativeInStream;

#define FWD_DEF_OUTSTREAM_OP(T)                                                \
    template<typename OStream>                                                 \
    inline bl::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>,     \
                           OStream>                                            \
      &operator<<(OStream &out, T value);

inline StreamResult
write_repr(OutStream &out, bl::string_view str);

inline StreamResult
write_repr(OutStream &out, const char *str);

inline StreamResult
write_repr(OutStream &out, char c);

inline StreamResult
write_repr(OutStream &out, signed char c);

inline StreamResult
write_repr(OutStream &out, unsigned char c);

inline StreamResult
write_repr(OutStream &out, const StreamEndl & /*unused*/);

#define FWD_DEF_OPAQUE_OUTSTREAM_OP(T)                                         \
    SYS_API StreamResult write_repr(OutStream &out, T x);                      \
    FWD_DEF_OUTSTREAM_OP(T)

FWD_DEF_OUTSTREAM_OP(char);
FWD_DEF_OUTSTREAM_OP(signed char);
FWD_DEF_OUTSTREAM_OP(unsigned char);
FWD_DEF_OUTSTREAM_OP(const char *);
FWD_DEF_OUTSTREAM_OP(bl::string_view);
FWD_DEF_OUTSTREAM_OP(const StreamEndl &);

FWD_DEF_OPAQUE_OUTSTREAM_OP(short)
FWD_DEF_OPAQUE_OUTSTREAM_OP(unsigned short)
FWD_DEF_OPAQUE_OUTSTREAM_OP(int)
FWD_DEF_OPAQUE_OUTSTREAM_OP(unsigned)
FWD_DEF_OPAQUE_OUTSTREAM_OP(long)
FWD_DEF_OPAQUE_OUTSTREAM_OP(unsigned long)
FWD_DEF_OPAQUE_OUTSTREAM_OP(long long)
FWD_DEF_OPAQUE_OUTSTREAM_OP(unsigned long long)

FWD_DEF_OPAQUE_OUTSTREAM_OP(float);
FWD_DEF_OPAQUE_OUTSTREAM_OP(double);
FWD_DEF_OPAQUE_OUTSTREAM_OP(long double);
FWD_DEF_OPAQUE_OUTSTREAM_OP(const void *);

#undef FWD_DEF_OPAQUE_OUTSTREAM_OP
#undef FWD_DEF_OUTSTREAM_OP

template<typename OStream>
inline bl::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream>
  &
  operator<<(OStream &out, bool x);

template<typename OStream>
inline bl::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream>
  &
  operator<<(OStream &out, const bl::string &str);

template<typename OStream>
inline bl::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream>
  &
  operator<<(OStream &out, char *str);

template<typename OStream, typename T>
inline bl::enable_if_t<std::is_base_of_v<sys::io::OutStream, OStream>, OStream>
  &
  operator<<(OStream &out, const T *ptr);

} // namespace io
} // namespace sys
#endif
