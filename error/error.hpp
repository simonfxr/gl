#ifndef ERROR_HPP
#define ERROR_HPP

#include "defs.h"

#include <string>
#include <sstream>

#ifndef ATTR_NORETURN
#include <cstdlib>
#endif

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
    Info,
    Warn,
    Error,
    ErrorOnce,
    DebugError,
    FatalError,
    DebugAssertion,
    Assertion
};

struct LogDestination {
    LogLevel minimumLevel;
    std::ostream& out;

    LogDestination(LogLevel minLvl, std::ostream& _out) :
        minimumLevel(minLvl), out(_out) {}
};

template <typename T>
struct LogTraits {
// static LogDestination getDestination(const T&);
};

template <typename T>
struct ErrorTraits {

    template <typename E>
    static void setError(T& value, const E& error) {
        value.pushError(error);
    }

    template <typename E>
    static std::string stringError(const E& error) {
        std::ostringstream rep;
        rep << error;
        return rep.str();
    }
};

template <typename T>
LogDestination getLogDestination(const T& v) {
    return LogTraits<T>::getDestination(v);
}

template <typename T, typename E>
void raiseError(const Location& loc, T& value, const E& error, LogLevel lvl, const std::string& message) {
    ErrorTraits<T>::setError(value, error);
    logRaiseError(loc, getLogDestination(value), lvl, ErrorTraits<T>::stringError(error), message);
}

void error(const Location& loc, LogLevel lvl, const std::string& mesg);

void error(const Location& loc, LogLevel lvl, const char *mesg);

void fatalError(const Location& loc, const std::string& mesg) ATTRS(ATTR_NORETURN);

void fatalError(const Location& loc, const char *mesg) ATTRS(ATTR_NORETURN);

void printError(std::ostream& out, const char *type, const Location& loc, LogLevel lvl, const std::string& mesg);

void printError(std::ostream& out, const char *type, const Location& loc, LogLevel lvl, const char *mesg);

std::ostream& logBegin(const Location& loc, const LogDestination&, LogLevel);

std::ostream& logWrite(const std::string&);

std::ostream& logWrite(const char *);

void logEnd();

void logRaiseError(const Location&, const LogDestination&, LogLevel, const std::string&, const std::string&);

#ifdef GNU_EXTENSIONS
#define _CURRENT_LOCATION_OP(op) ::err::Location(__FILE__, __LINE__, __PRETTY_FUNCTION__, op)
#else
#define _CURRENT_LOCATION_OP(op) ::err::Location(__FILE__, __LINE__, __FUNCTION__, op)
#endif

#define _CURRENT_LOCATION _CURRENT_LOCATION_OP(0)

#define _ERROR(lvl, msg) ::err::error(_CURRENT_LOCATION, lvl, (msg))
#define _ASSERT(x, lvl, msg)  do { if (unlikely(!(x))) _ERROR(lvl, msg); } while (0)
#define _ASSERT_EXPR(x, lvl, msg, expr) ((unlikely(!(x)) ? (_ERROR(lvl, msg), 0) : 0), (expr))

#ifdef DEBUG
#define DEBUG_ASSERT_MSG(x, msg) _ASSERT(x, ::err::DebugAssertion, msg)
#define DEBUG_ERR(msg) _ERROR(::err::DebugError, msg)
#else
#define DEBUG_ASSERT_MSG(x, msg) UNUSED(0)
#define DEBUG_ERR(msg) UNUSED(0)
#endif

#define DEBUG_ASSERT(x) DEBUG_ASSERT_MSG(x, AS_STR(x))

#define ASSERT_MSG(x, msg) _ASSERT(x, ::err::Assertion, msg)
#define ASSERT(x) ASSERT_MSG(x, AS_STR(x))
#define ASSERT_MSG_EXPR(x, msg, expr) _ASSERT_EXPR(x, ::err::Assertion, msg, expr)
#define ASSERT_EXPR(x, expr) ASSERT_MSG_EXPR(x, AS_STR(x), expr)

#define ERR(msg) _ERROR(::err::Error, msg)
#define WARN(msg) _ERROR(::err::Warn, msg)

#ifdef ATTR_NORETURN
#define FATAL_ERR(msg) ::err::fatalError(_CURRENT_LOCATION, msg)
#else
#define FATAL_ERR(msg) (::err::fatalError(_CURRENT_LOCATION, msg), ::exit(1) /* exit never reached */)
#endif

#define ERR_ONCE(msg) do {                                      \
        static bool _reported = false;                          \
        if (unlikely(!_reported)) {                             \
            _reported = true;                                   \
            _ERROR(::err::ErrorOnce, msg);                      \
        }                                                       \
    } while (0)

#define LOG_BEGIN(val, lvl) ::err::logBegin(_CURRENT_LOCATION, ::err::getLogDestination((val)), (lvl));
#define LOG_END(val) (UNUSED(val), ::err::logEnd())
#define LOG_RAISE(val, err, lvl, msg) ::err::raiseError(_CURRENT_LOCATION, (val), (err), (lvl), msg)
#define LOG_PUT(msg) ::err::logWrite((msg))

} // namespace err
#endif
