#ifndef ERROR_HPP
#define ERROR_HPP

#include "err/conf.hpp"
#include "sys/io/Stream.hpp"

#include <string_view>

namespace err {

ERR_API void
print_stacktrace(sys::io::OutStream &, int skip = 0);

struct Location
{
    const char *file;
    const char *function;
    const char *operation; // may be null
    int line;

    constexpr Location(const char *fl,
                       int ln,
                       const char *fn,
                       const char *op = nullptr) noexcept
      : file(fl), function(fn), operation(op), line(ln)
    {}
};

enum class LogLevel : uint8_t
{
    Info,
    Warn,
    Error,
    ErrorOnce,
    FatalError,
    Assertion
};

struct ErrorStaticCallSite
{
    Location location;
    LogLevel level;
    const char *message;
    constexpr ErrorStaticCallSite(const Location &loc,
                                  LogLevel lvl,
                                  const char *msg) noexcept
      : location(loc), level(lvl), message(msg)
    {}

    // the following constructors will not be called, they are necessary to make
    // the __builtin_constant_p machinery work

    ErrorStaticCallSite(const Location &loc,
                        LogLevel lvl,
                        std::string_view msg) noexcept;

    ErrorStaticCallSite(const Location &loc,
                        LogLevel lvl,
                        sys::io::OutStream &,
                        std::string_view msg) noexcept;
};

// we have some nearly identical overloads, this is because we try to generate
// the minimal amount of code for calls to such functions. Taking pointers
// instead of const refernces to Location's convinces gcc/clang to move those
// arguments into the static data/readonly section

ERR_API void
error(const Location *loc, LogLevel lvl, std::string_view);

ERR_API void
error(const Location *loc,
      LogLevel lvl,
      sys::io::OutStream &,
      std::string_view);

ERR_API void
error(const ErrorStaticCallSite *);

HU_NORETURN ERR_API void
fatalError(const Location *loc, LogLevel lvl, std::string_view);

HU_NORETURN ERR_API void
fatalError(const Location *loc,
           LogLevel lvl,
           sys::io::OutStream &,
           std::string_view);

HU_NORETURN ERR_API void
fatalError(const ErrorStaticCallSite *);

ERR_API void
reportError(sys::io::OutStream &out,
            const char *type,
            const Location &loc,
            LogLevel lvl,
            std::string_view);

#ifdef HU_PRETTY_FUNCTION
#define ERR_FUNCTION HU_PRETTY_FUNCTION
#else
#define ERR_FUNCTION __func__
#endif

#define ERROR_LOCATION_OP_BASIC(op)                                            \
    ::err::Location(__FILE__, __LINE__, ERR_FUNCTION, op)

#if HU_COMP_GNULIKE_P
#define ERROR_LOCATION_OP(op)                                                  \
    ({                                                                         \
        static constexpr auto PP_CAT(err_loc, __LINE__) =                      \
          ERROR_LOCATION_OP_BASIC(op);                                         \
        &PP_CAT(err_loc, __LINE__);                                            \
    })
#else
#define ERROR_LOCATION_OP(op) &ERROR_LOCATION_OP_BASIC(op)
#endif

#define ERROR_LOCATION ERROR_LOCATION_OP(nullptr)

#define CALL_ERROR_WITH_LEVEL_BASIC(errfun, lvl, op, ...)                      \
    errfun(ERROR_LOCATION_OP(op), lvl, __VA_ARGS__)

#if HU_HAVE_constant_p && HU_COMP_GNULIKE_P
#define CALL_ERROR_WITH_LEVEL_CHECK_CONSTANT(a, b, ...)                        \
    hu_constant_p(a) && hu_constant_p(b)
#define CALL_ERROR_WITH_LEVEL(errfun, lvl, op, ...)                            \
    do {                                                                       \
        if (hu_constant_p(lvl) &&                                              \
            CALL_ERROR_WITH_LEVEL_CHECK_CONSTANT(__VA_ARGS__, 0)) {            \
            static const auto PP_CAT(_fatal_error_static_callsite_,            \
                                     __LINE__) =                               \
              ::err::ErrorStaticCallSite(                                      \
                ERROR_LOCATION_OP_BASIC(op), lvl, __VA_ARGS__);                \
            errfun(&PP_CAT(_fatal_error_static_callsite_, __LINE__));          \
        } else {                                                               \
            CALL_ERROR_WITH_LEVEL_BASIC(errfun, lvl, op, __VA_ARGS__);         \
        }                                                                      \
    } while (0)
#else
#define CALL_ERROR_WITH_LEVEL(errfun, lvl, op, ...)                            \
    CALL_ERROR_WITH_LEVEL_BASIC(errfun, lvl, op, __VA_ARGS__)
#endif

#define ERROR_WITH_LEVEL(lvl, ...)                                             \
    CALL_ERROR_WITH_LEVEL(::err::error, lvl, nullptr, __VA_ARGS__)

#define FATAL_ERROR_WITH_LEVEL_AND_OP(lvl, op, ...)                            \
    CALL_ERROR_WITH_LEVEL(::err::fatalError, lvl, op, __VA_ARGS__)

#define FATAL_ERROR_WITH_LEVEL(lvl, ...)                                       \
    FATAL_ERROR_WITH_LEVEL_AND_OP(lvl, nullptr, __VA_ARGS__)

#define ERR(...) ERROR_WITH_LEVEL(::err::LogLevel::Error, __VA_ARGS__)
#define WARN(...) ERROR_WITH_LEVEL(::err::LogLevel::Warn, __VA_ARGS__)
#define INFO(...) ERROR_WITH_LEVEL(::err::LogLevel::Info, __VA_ARGS__)
#define FATAL_ERR(...)                                                         \
    FATAL_ERROR_WITH_LEVEL(::err::LogLevel::FatalError, __VA_ARGS__)

#define ASSERT_ALWAYS(...)                                                     \
    do {                                                                       \
        if (unlikely(!(PP_ARG1(__VA_ARGS__))))                                 \
            FATAL_ERROR_WITH_LEVEL_AND_OP(                                     \
              ::err::LogLevel::Assertion,                                      \
              PP_TOSTR(PP_ARG1(__VA_ARGS__)),                                  \
              PP_ARG2(__VA_ARGS__, "assertion failed"));                       \
    } while (0)

#ifdef NDEBUG
#define ASSERT(cond, ...) hu_assume(cond)
#define UNREACHABLE_MSG(msg) hu_assume_unreachable()
#define DEBUG_ERR(...) ((void) 0)
#define DEBUG_ASSERT(...) ((void) 0)
#define TRACE(...)                                                             \
    do {                                                                       \
        __VA_ARGS__;                                                           \
    } while (0)
#else
#define ASSERT(...) ASSERT_ALWAYS(__VA_ARGS__)
#define UNREACHABLE_MSG(msg)                                                   \
    FATAL_ERROR_WITH_LEVEL_AND_OP(::err::LogLevel::Assertion, nullptr, msg)
#define DEBUG_ERR(...) ERR(__VA_ARGS__)
#define DEBUG_ASSERT(...) ASSERT(__VA_ARGS__)
#define TRACE(...)                                                             \
    do {                                                                       \
        INFO(HU_TOSTR(__VA_ARGS__));                                           \
        __VA_ARGS__;                                                           \
    } while (0)
#endif

#define UNREACHABLE UNREACHABLE_MSG("assumed to be unreachable")

#define ERR_ONCE(...)                                                          \
    do {                                                                       \
        static const bool PP_CAT(_err_once_reported_, __LINE__) = [&]() {      \
            ERROR_WITH_LEVEL(::err::LogLevel::ErrorOnce, __VA_ARGS__);         \
            return true;                                                       \
        }();                                                                   \
        UNUSED(PP_CAT(_err_once_reported_, __LINE__));                         \
    } while (0)

} // namespace err
#endif
