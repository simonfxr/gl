#ifndef ERROR_HPP
#define ERROR_HPP

#include "defs.h"

#include <string>

namespace err {

struct Location {
    const char *file;
    int line;
    const char *function;
    const char *operation; // may be null
    Location(const char *_file, int _line, const char *_function, const char *_operation = 0) :
        file(_file), line(_line), function(_function), operation(_operation) {}
};

enum LogLevel {
    DebugError,
    DebugAssertion,
    Warn,
    Assertion,
    Error,
    ErrorOnce,
    FatalError
};

void error(const Location& loc, LogLevel lvl, const std::string& mesg);

void error(const Location& loc, LogLevel lvl, const char *mesg);

void fatalError(const Location& loc, const std::string& mesg) ATTRS(ATTR_NORETURN);

void fatalError(const Location& loc, const char *mesg) ATTRS(ATTR_NORETURN);

void printError(std::ostream& out, const char *type, const Location& loc, LogLevel lvl, const std::string& mesg);

void printError(std::ostream& out, const char *type, const Location& loc, LogLevel lvl, const char *mesg);

#ifdef GNU_EXTENSIONS
#define _CURRENT_LOCATION_OP(op) err::Location(__FILE__, __LINE__, __PRETTY_FUNCTION__, op)
#else
#define _CURRENT_LOCATION_OP(op) err::Location(__FILE__, __LINE__, __FUNCTION__, op)
#endif

#define _CURRENT_LOCATION _CURRENT_LOCATION_OP(0)

#define _ERROR(lvl, msg) err::error(_CURRENT_LOCATION, lvl, (msg))
#define _ASSERT(x, lvl, msg)  do { if (unlikely(!(x))) _ERROR(lvl, msg); } while (0)
#define _ASSERT_EXPR(x, lvl, msg, expr) ((unlikely(!(x)) ? (_ERROR(lvl, msg), 0) : 0), (expr))

#ifdef DEBUG
#define DEBUG_ASSERT_MSG(x, msg) _ASSERT(x, err::DebugAssertion, msg)
#define DEBUG_ERR(msg) _ERROR(err::DebugError, msg)
#else
#define DEBUG_ASSERT_MSG(x, msg) UNUSED(0)
#define DEBUG_ERR(msg) UNUSED(0)
#endif

#define DEBUG_ASSERT(x) DEBUG_ASSERT_MSG(x, AS_STR(x))

#define ASSERT_MSG(x, msg) _ASSERT(x, err::Assertion, msg)
#define ASSERT(x) ASSERT_MSG(x, AS_STR(x))
#define ASSERT_MSG_EXPR(x, msg, expr) _ASSERT_EXPR(x, err::Assertion, msg, expr)
#define ASSERT_EXPR(x, expr) ASSERT_MSG_EXPR(x, AS_STR(x), expr)

#define ERR(msg) _ERROR(err::Error, msg)
#define WARN(msg) _ERROR(err::Warn, msg)
#define FATAL_ERR(msg) err::fatalError(_CURRENT_LOCATION, msg)

#define ERR_ONCE(msg) do {                                      \
        static bool _reported = false;                          \
        if (unlikely(!_reported)) {                             \
            _reported = true;                                   \
            _ERROR(err::ErrorOnce, msg);                        \
        }                                                       \
    } while (0)

} // namespace err
#endif
