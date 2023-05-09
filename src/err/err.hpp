#ifndef ERROR_HPP
#define ERROR_HPP

#include "err/conf.hpp"
#include "sys/io/Stream.hpp"

#include <bl/string_view.hpp>

namespace err {

ERR_API void
print_stacktrace(sys::io::OutStream, int skip = 0);

struct string_arg
{
    bl::string_view str;

    template<typename T>
        requires(std::convertible_to<T, bl::string_view>)
    constexpr /* implicit */ string_arg(T &&arg)
      : str(static_cast<bl::string_view>(bl_fwd(arg)))
    {}

    template<size_t N>
        requires(N > 0)
    BL_consteval /* implicit */ string_arg(const char (&arg)[N])
      : str(bl::string_view_literal(arg))
    {}

    constexpr string_arg() = default;
};

struct Location
{
    const char *file;
    const char *function;
    bl::string_view operation;
    int line;

    BL_consteval Location(const char *fl,
                          int ln,
                          const char *fn,
                          string_arg op = {}) noexcept
      : file(fl), function(fn), operation(op.str), line(ln)
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
    bl::string_view message;

    constexpr ErrorStaticCallSite(const Location &loc,
                                  LogLevel lvl,
                                  string_arg msg) noexcept
      : location(loc), level(lvl), message(msg.str)
    {}

    // unimplemeneted, and never called. It is required to make the
    // builtin_choose_expr machinery work.
    ErrorStaticCallSite(const Location &loc,
                        LogLevel lvl,
                        sys::io::OutStream,
                        string_arg msg) noexcept;
};

namespace internal {

// we have some nearly identical overloads, this is because we try to generate
// the minimal amount of code for calls to such functions. Taking pointers
// instead of const refernces to Location's convinces gcc/clang to move those
// arguments into the static data/readonly section

static inline constexpr struct error_f
{

    ERR_API static void apply(const Location *loc,
                              LogLevel lvl,
                              string_arg) noexcept;

    ERR_API static void apply(const Location *loc,
                              LogLevel lvl,
                              sys::io::OutStream,
                              string_arg) noexcept;

    ERR_API static void apply(const ErrorStaticCallSite *) noexcept;

} error;

static inline constexpr struct fatalError_f
{

    HU_NORETURN ERR_API static void apply(const Location *loc,
                                          LogLevel lvl,
                                          string_arg) noexcept;

    HU_NORETURN ERR_API static void apply(const Location *loc,
                                          LogLevel lvl,
                                          sys::io::OutStream,
                                          string_arg) noexcept;

    HU_NORETURN ERR_API static void apply(const ErrorStaticCallSite *) noexcept;

} fatalError;

} // namespace internal

ERR_API void
reportError(sys::io::OutStream out,
            const char *type,
            const Location &loc,
            LogLevel lvl,
            bl::string_view);

#ifdef HU_PRETTY_FUNCTION
#    define ERR_FUNCTION HU_PRETTY_FUNCTION
#else
#    define ERR_FUNCTION __func__
#endif

#define ERROR_LOCATION_OP_BASIC(op)                                            \
    ::err::Location(__FILE__, __LINE__, ERR_FUNCTION, op)

#if HU_COMP_INTEL_P
#    define ERROR_LOCATION_OP(op)                                              \
        [&] {                                                                  \
            static constexpr auto PP_CAT(err_loc, __LINE__) =                  \
              ERROR_LOCATION_OP_BASIC(op);                                     \
            return &PP_CAT(err_loc, __LINE__);                                 \
        }()

#elif HU_COMP_GNULIKE_P
#    define ERROR_LOCATION_OP(op)                                              \
        __extension__({                                                        \
            static constexpr auto PP_CAT(err_loc, __LINE__) =                  \
              ERROR_LOCATION_OP_BASIC(op);                                     \
            &PP_CAT(err_loc, __LINE__);                                        \
        })

#else
#    define ERROR_LOCATION_OP(op) &ERROR_LOCATION_OP_BASIC(op)
#endif

#define ERROR_LOCATION ERROR_LOCATION_OP("")

#define CALL_ERROR_WITH_LEVEL_BASIC(errfun, lvl, op, ...)                      \
    errfun.apply(ERROR_LOCATION_OP(op), lvl, __VA_ARGS__)

#define CALL_ERROR_WITH_LEVEL_STATIC(callsite_name, errfun, lvl, op, ...)      \
    __extension__({                                                            \
        static const auto callsite_name = ::err::ErrorStaticCallSite(          \
          ERROR_LOCATION_OP_BASIC(op), lvl, __VA_ARGS__);                      \
        errfun.apply(&callsite_name);                                          \
    })

#if HU_HAVE_constant_p && HU_COMP_GNULIKE_P && !HU_COMP_INTEL_P

#    if hu_has_builtin(__builtin_choose_expr)
#        define ERR_CHOOSE_EXPR __builtin_choose_expr
#    else
#        define ERR_CHOOSE_EXPR(p, a, b) ((p) ? (a) : (b))
#    endif

#    define CALL_ERROR_WITH_LEVEL_CHECK_CONSTANT(a, b, ...)                    \
        hu_constant_p(a) && hu_constant_p(b)

#    define CALL_ERROR_WITH_LEVEL(errfun, lvl, op, ...)                        \
        ERR_CHOOSE_EXPR(                                                       \
          hu_constant_p(lvl) &&                                                \
            CALL_ERROR_WITH_LEVEL_CHECK_CONSTANT(__VA_ARGS__, 0),              \
          CALL_ERROR_WITH_LEVEL_STATIC(HU_CAT_COUNTER(_error_static_callsite), \
                                       errfun,                                 \
                                       lvl,                                    \
                                       op,                                     \
                                       __VA_ARGS__),                           \
          CALL_ERROR_WITH_LEVEL_BASIC(errfun, lvl, op, __VA_ARGS__))

#else
#    define CALL_ERROR_WITH_LEVEL(errfun, lvl, op, ...)                        \
        CALL_ERROR_WITH_LEVEL_BASIC(errfun, lvl, op, __VA_ARGS__)
#endif

#define ERROR_WITH_LEVEL(lvl, ...)                                             \
    CALL_ERROR_WITH_LEVEL(::err::internal::error, lvl, "", __VA_ARGS__)

#define FATAL_ERROR_WITH_LEVEL_AND_OP(lvl, op, ...)                            \
    CALL_ERROR_WITH_LEVEL(::err::internal::fatalError, lvl, op, __VA_ARGS__)

#define FATAL_ERROR_WITH_LEVEL(lvl, ...)                                       \
    FATAL_ERROR_WITH_LEVEL_AND_OP(lvl, "", __VA_ARGS__)

#define ERR(...) ERROR_WITH_LEVEL(::err::LogLevel::Error, __VA_ARGS__)
#define WARN(...) ERROR_WITH_LEVEL(::err::LogLevel::Warn, __VA_ARGS__)
#define INFO(...) ERROR_WITH_LEVEL(::err::LogLevel::Info, __VA_ARGS__)
#define FATAL_ERR(...)                                                         \
    FATAL_ERROR_WITH_LEVEL(::err::LogLevel::FatalError, __VA_ARGS__)

#define ASSERT_ALWAYS(...)                                                     \
    (unlikely(!(PP_ARG1(__VA_ARGS__)))                                         \
       ? (void) FATAL_ERROR_WITH_LEVEL_AND_OP(                                 \
           ::err::LogLevel::Assertion,                                         \
           ::bl::string_view_literal(PP_TOSTR(PP_ARG1(__VA_ARGS__))),          \
           PP_ARG2(__VA_ARGS__, "assertion failed"))                           \
       : (void) 0)

#ifdef NDEBUG
#    define ASSERT(cond, ...) hu_assume(cond)
#    define UNREACHABLE_MSG(msg) hu_unreachable()
#    define DEBUG_ERR(...) ((void) 0)
#    define DEBUG_ASSERT(...) ((void) 0)
#    define TRACE(...) (__VA_ARGS__)

#else
#    define ASSERT(...) ASSERT_ALWAYS(__VA_ARGS__)
#    define UNREACHABLE_MSG(msg)                                               \
        FATAL_ERROR_WITH_LEVEL_AND_OP(::err::LogLevel::Assertion, "", msg)
#    define DEBUG_ERR(...) ERR(__VA_ARGS__)
#    define DEBUG_ASSERT(...) ASSERT(__VA_ARGS__)
#    define TRACE(...) (INFO(HU_TOSTR(__VA_ARGS__)), (__VA_ARGS__))

#endif

#define UNREACHABLE UNREACHABLE_MSG("assumed to be unreachable")

#define ERR_ONCE_(id, ...)                                                     \
    (likely([]() {                                                             \
        static volatile bool id = false;                                       \
        return ::bl::exchange(id, true);                                       \
    }())                                                                       \
       ? (void) 0                                                              \
       : ERR(__VA_ARGS__))

#define ERR_ONCE(...) ERR_ONCE_(HU_CAT_COUNTER(_err_once_reported), __VA_ARGS__)

} // namespace err

#endif
