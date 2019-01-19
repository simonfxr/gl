#ifndef ERROR_HPP
#define ERROR_HPP

#include "err/conf.hpp"
#include "sys/io/Stream.hpp"

#include <string>
#include <string_view>

#ifndef ERROR_DEFAULT_STREAM
#define ERROR_DEFAULT_STREAM ::sys::io::stdout()
#endif

#include "err/WithError.hpp"

template<typename T>
void
to_string(T &&) = delete;

template<typename T>
struct LogTraits
{
    // static err::LogDestination getDestination(const T&);
};

template<typename T>
struct ErrorTraits
{
    template<typename E, typename Arg>
    static void setError(err::WithError<E> &x, Arg &&err)
    {
        x.pushError(std::forward<Arg>(err));
    }
};

namespace err {

ERR_API void
print_stacktrace(sys::io::OutStream &, int skip = 0);

struct Location
{
    const char *file;
    const char *function;
    const char *operation; // may be null
    int line;

    static constexpr Location make(const char *_file,
                                   int _line,
                                   const char *_function,
                                   const char *_operation = nullptr) noexcept
    {
        return { _file, _function, _operation, _line };
    }
};

enum class LogLevel : uint8_t
{
    Info,
    Warn,
    Error,
    ErrorOnce,
    DebugError,
    FatalError,
    DebugAssertion,
    Assertion
};

struct LogDestination
{
    LogLevel minimumLevel;
    sys::io::OutStream &out;

    constexpr LogDestination(LogLevel minLvl, sys::io::OutStream &_out) noexcept
      : minimumLevel(minLvl), out(_out)
    {}
};

struct ErrorArgs
{
    sys::io::OutStream &out;
    std::string_view mesg;

    constexpr ErrorArgs(sys::io::OutStream &o, std::string_view msg) noexcept
      : out(o), mesg(msg)
    {}

#ifndef ERROR_NO_IMPLICIT_OUT
    ErrorArgs(std::string_view msg) noexcept
      : out(ERROR_DEFAULT_STREAM), mesg(msg)
    {}
#endif
};

ERR_API void
error(const Location &loc, LogLevel lvl, const ErrorArgs &);

HU_NORETURN ERR_API void
fatalError(const Location &loc, LogLevel lvl, const ErrorArgs &);

ERR_API void
printError(sys::io::OutStream &out,
           const char *type,
           const Location &loc,
           LogLevel lvl,
           std::string_view);

ERR_API sys::io::OutStream &
logBegin(const Location &loc, const LogDestination &, LogLevel);

ERR_API sys::io::OutStream &logWrite(std::string_view);

ERR_API sys::io::OutStream &logWriteErr(std::string_view, std::string_view);

ERR_API void
logEnd();

ERR_API void
logRaiseError(const Location &,
              const LogDestination &,
              LogLevel,
              std::string_view,
              std::string_view);

template<typename T>
constexpr LogDestination
getLogDestination(const T &v)
{
    return LogTraits<T>::getDestination(v);
}

template<typename T, typename E>
void
logRaiseError(const Location &loc,
              T &value,
              E error,
              LogLevel lvl,
              std::string_view message)
{
    ErrorTraits<T>::setError(value, error);
    using ::to_string;
    logRaiseError(
      loc, getLogDestination(value), lvl, to_string(error), message);
}

template<typename T>
constexpr bool
logLevel(const T &value, LogLevel lvl)
{
    return lvl >= getLogDestination(value).minimumLevel;
}

template<typename T>
constexpr sys::io::OutStream &
logDestination(const T &value)
{
    return getLogDestination(value).out;
}

template<typename T, typename E>
sys::io::OutStream &
logPutError(const T &, E err, std::string_view msg)
{
    using ::to_string;
    return logWriteErr(to_string(err), msg);
}

#ifdef HU_PRETTY_FUNCTION
#define ERR_FUNCTION HU_PRETTY_FUNCTION
#else
#define ERR_FUNCTION __func__
#endif

#define DETAIL_CURRENT_LOCATION_OP_BASIC(op)                                   \
    ::err::Location::make(__FILE__, __LINE__, ERR_FUNCTION, op)

#if defined(hu_constant_p) && HU_COMP_GNULIKE_P
#define DETAIL_CURRENT_LOCATION_OP_STATIC(op)                                  \
    ({                                                                         \
        static constexpr auto PP_CAT(err_loc, __LINE__) =                      \
          DETAIL_CURRENT_LOCATION_OP_BASIC(op);                                \
        PP_CAT(err_loc, __LINE__);                                             \
    })

#define DETAIL_CURRENT_LOCATION_OP(op)                                         \
    (hu_constant_p(op) ? DETAIL_CURRENT_LOCATION_OP_STATIC(op)                 \
                       : DETAIL_CURRENT_LOCATION_OP_BASIC(op))
#define DETAIL_CURRENT_LOCATION DETAIL_CURRENT_LOCATION_OP_STATIC(nullptr)

#else

#define DETAIL_CURRENT_LOCATION_OP DETAIL_CURRENT_LOCATION_OP_BASIC
#define DETAIL_CURRENT_LOCATION DETAIL_CURRENT_LOCATION_OP(nullptr)

#endif

#define DETAIL_ERROR(lvl, ...)                                                 \
    ::err::error(DETAIL_CURRENT_LOCATION, lvl, ::err::ErrorArgs(__VA_ARGS__))
#define DETAIL_ASSERT(x, lvl, msg)                                             \
    do {                                                                       \
        if (unlikely(!(x)))                                                    \
            DETAIL_ERROR(lvl, ERROR_DEFAULT_STREAM, msg);                      \
    } while (0)
#define DETAIL_ASSERT_EXPR(x, lvl, msg, expr)                                  \
    ((unlikely(!(x)) ? (DETAIL_ERROR(lvl, ERROR_DEFAULT_STREAM, msg), 0) : 0), \
     (expr))

#ifdef DEBUG
#define DEBUG_ASSERT_MSG(x, msg)                                               \
    DETAIL_ASSERT(x, ::err::LogLevel::DebugAssertion, msg)
#define DEBUG_ERR(...) DETAIL_ERROR(::err::LogLevel::DebugError, __VA_ARGS__)
#else
#define DEBUG_ASSERT_MSG(x, msg) UNUSED(0)
#define DEBUG_ERR(msg) UNUSED(0)
#endif

#define DEBUG_ASSERT(x) DEBUG_ASSERT_MSG(x, PP_TOSTR(x))

#define ASSERT_MSG(x, msg) DETAIL_ASSERT(x, ::err::LogLevel::Assertion, msg)
#define ASSERT(x) ASSERT_MSG(x, PP_TOSTR(x))
#define ASSERT_MSG_EXPR(x, msg, expr)                                          \
    DETAIL_ASSERT_EXPR(x, ::err::LogLevel::Assertion, msg, expr)
#define ASSERT_EXPR(x, expr) ASSERT_MSG_EXPR(x, PP_TOSTR(x), expr)

#define ERR(...) DETAIL_ERROR(::err::LogLevel::Error, __VA_ARGS__)
#define WARN(...) DETAIL_ERROR(::err::LogLevel::Warn, __VA_ARGS__)
#define INFO(...) DETAIL_ERROR(::err::LogLevel::Info, __VA_ARGS__)

#if HU_HAVE_NORETURN_P
#define DETAIL_FATAL_ERR(lvl, ...)                                             \
    ::err::fatalError(                                                         \
      DETAIL_CURRENT_LOCATION, lvl, ::err::ErrorArgs(__VA_ARGS__))
#else
#define DETAIL_FATAL_ERR(lvl, ...)                                             \
    (::err::fatalError(                                                        \
       DETAIL_CURRENT_LOCATION, lvl, ::err::ErrorArgs(__VA_ARGS__)),           \
     ::abort())
#endif

#define FATAL_ERR(...)                                                         \
    DETAIL_FATAL_ERR(::err::LogLevel::FatalError, __VA_ARGS__)

#define ASSERT_FAIL_MSG(msg)                                                   \
    DETAIL_FATAL_ERR(::err::LogLevel::Assertion, ERROR_DEFAULT_STREAM, msg)

#define ASSERT_FAIL() ASSERT_FAIL_MSG("unreachable")

#define ERR_ONCE(...)                                                          \
    do {                                                                       \
        static const bool _reported = [&]() {                                  \
            DETAIL_ERROR(::err::LogLevel::ErrorOnce, __VA_ARGS__);             \
            return true;                                                       \
        }();                                                                   \
        UNUSED(_reported);                                                     \
    } while (0)

#define LOG_BEGIN(val, lvl)                                                    \
    ::err::logBegin(                                                           \
      DETAIL_CURRENT_LOCATION, ::err::getLogDestination((val)), (lvl));
#define LOG_END(val) (UNUSED(val), ::err::logEnd())
#define LOG_RAISE(val, ec, lvl, msg)                                           \
    ::err::logRaiseError(DETAIL_CURRENT_LOCATION, (val), (ec), (lvl), msg)
#define LOG_PUT(val, msg) (UNUSED(val), ::err::logWrite((msg)))
#define LOG_PUT_ERR(val, ec, msg) ::err::logPutError(val, ec, msg)
#define LOG_LEVEL(val, lvl) ::err::logLevel(val, lvl)
#define LOG_DESTINATION(val) ::err::logDestination(val)

#define TRACE(...) INFO(__VA_ARGS__)

} // namespace err
#endif
