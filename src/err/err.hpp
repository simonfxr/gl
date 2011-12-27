#ifndef ERROR_HPP
#define ERROR_HPP

#include "defs.hpp"

#include "sys/io/Stream.hpp"

#include <string>
#include <sstream>

#ifndef ATTR_NORETURN
#include <stdlib.h>
#endif

#ifndef ERROR_DEFAULT_STREAM
#define ERROR_DEFAULT_STREAM ::sys::io::stdout()
#endif

#include "err/WithError.hpp"

template <typename T>
struct LogTraits {
// static err::LogDestination getDestination(const T&);
};

template <typename T>
struct ErrorTraits {
    
    template <typename E, E NoError, std::string(*StringError)(E)>
    static std::string stringError(const err::WithError<E, NoError, StringError>&, E err) {
        return err::WithError<E, NoError, StringError>::stringError(err);
    }
    
    template <typename E, E NoError, std::string(*StringError)(E)>
    static void setError(err::WithError<E, NoError, StringError>& x, E err) {
        x.pushError(err);
    }
};

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
    sys::io::OutStream& out;

    LogDestination(LogLevel minLvl, sys::io::OutStream& _out) :
        minimumLevel(minLvl), out(_out) {}
};

void error(const Location& loc, sys::io::OutStream&, LogLevel lvl, const std::string& mesg);

void error(const Location& loc, sys::io::OutStream&, LogLevel lvl, const char *mesg);

void fatalError(const Location& loc, sys::io::OutStream&, LogLevel lvl, const std::string& mesg) ATTRS(ATTR_NORETURN);

void fatalError(const Location& loc, sys::io::OutStream&, LogLevel lvl, const char *mesg) ATTRS(ATTR_NORETURN);

void printError(sys::io::OutStream& out, const char *type, const Location& loc, LogLevel lvl, const std::string& mesg);

void printError(sys::io::OutStream& out, const char *type, const Location& loc, LogLevel lvl, const char *mesg);

sys::io::OutStream& logBegin(const Location& loc, const LogDestination&, LogLevel);

sys::io::OutStream& logWrite(const std::string&);

sys::io::OutStream& logWriteErr(const std::string&, const std::string&);

sys::io::OutStream& logWrite(const char *);

void logEnd();

void logRaiseError(const Location&, const LogDestination&, LogLevel, const std::string&, const std::string&);

template <typename T>
LogDestination getLogDestination(const T& v) {
    return LogTraits<T>::getDestination(v);
}

template <typename T, typename E>
void logRaiseError(const Location& loc, T& value, E error, LogLevel lvl, const std::string& message) {
    ErrorTraits<T>::setError(value, error);
    logRaiseError(loc, getLogDestination(value), lvl, ErrorTraits<T>::stringError(value, error), message);
}

template <typename T>
bool logLevel(const T& value, LogLevel lvl) {
    return lvl >= getLogDestination(value).minimumLevel;
}

template <typename T>
sys::io::OutStream& logDestination(const T& value) {
    return getLogDestination(value).out;
}

template <typename T, typename E>
sys::io::OutStream& logPutError(const T& v, E err, const std::string& msg) {
    return logWriteErr(ErrorTraits<T>::stringError(v, err), msg);
}

#ifdef GNU_EXTENSIONS
#define _CURRENT_LOCATION_OP(op) ::err::Location(__FILE__, __LINE__, __PRETTY_FUNCTION__, op)
#else
#define _CURRENT_LOCATION_OP(op) ::err::Location(__FILE__, __LINE__, __FUNCTION__, op)
#endif

#define _CURRENT_LOCATION _CURRENT_LOCATION_OP(0)

#define _ERROR(lvl, msg) ::err::error(_CURRENT_LOCATION, ERROR_DEFAULT_STREAM, lvl, (msg))
#define ___ASSERT(x, lvl, msg)  do { if (unlikely(!(x))) _ERROR(lvl, msg); } while (0)
#define ___ASSERT_EXPR(x, lvl, msg, expr) ((unlikely(!(x)) ? (_ERROR(lvl, msg), 0) : 0), (expr))

#ifdef DEBUG
#define DEBUG_ASSERT_MSG(x, msg) ___ASSERT(x, ::err::DebugAssertion, msg)
#define DEBUG_ERR(msg) _ERROR(::err::DebugError, msg)
#else
#define DEBUG_ASSERT_MSG(x, msg) UNUSED(0)
#define DEBUG_ERR(msg) UNUSED(0)
#endif

#define DEBUG_ASSERT(x) DEBUG_ASSERT_MSG(x, AS_STR(x))

#define ASSERT_MSG(x, msg) ___ASSERT(x, ::err::Assertion, msg)
#define ASSERT(x) ASSERT_MSG(x, AS_STR(x))
#define ASSERT_MSG_EXPR(x, msg, expr) ___ASSERT_EXPR(x, ::err::Assertion, msg, expr)
#define ASSERT_EXPR(x, expr) ASSERT_MSG_EXPR(x, AS_STR(x), expr)

#define ERR(msg) _ERROR(::err::Error, msg)
#define WARN(msg) _ERROR(::err::Warn, msg)
#define INFO(msg) _ERROR(::err::Info, msg)

#ifdef ATTR_NORETURN
#define _FATAL_ERR(msg, lvl) ::err::fatalError(_CURRENT_LOCATION, ERROR_DEFAULT_STREAM, lvl, msg)
#else
#define _FATAL_ERR(msg, lvl) (::err::fatalError(_CURRENT_LOCATION, ERROR_DEFAULT_STREAM, lvl, msg), ::exit(1) /* exit never reached */)
#endif

#define FATAL_ERR(msg) _FATAL_ERR(msg, ::err::FatalError)

#define ASSERT_FAIL_MSG(msg) _FATAL_ERR(msg, ::err::Assertion)

#define ASSERT_FAIL() ASSERT_FAIL_MSG("unreachable")

#define ERR_ONCE(msg) do {                                      \
        static bool _reported = false;                          \
        if (unlikely(!_reported)) {                             \
            _reported = true;                                   \
            _ERROR(::err::ErrorOnce, msg);                      \
        }                                                       \
    } while (0)

#define LOG_BEGIN(val, lvl) ::err::logBegin(_CURRENT_LOCATION, ::err::getLogDestination((val)), (lvl));
#define LOG_END(val) (UNUSED(val), ::err::logEnd())
#define LOG_RAISE(val, ec, lvl, msg) ::err::logRaiseError(_CURRENT_LOCATION, (val), (ec), (lvl), msg)
#define LOG_PUT(val, msg) (UNUSED(val), ::err::logWrite((msg)))
#define LOG_PUT_ERR(val, ec, msg) ::err::logPutError(val, ec, msg)
#define LOG_LEVEL(val, lvl) ::err::logLevel(val, lvl)
#define LOG_DESTINATION(val) ::err::logDestination(val)

} // namespace err
#endif
