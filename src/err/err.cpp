#include "err/err.hpp"

#include "err/stacktrace.hpp"
#include "sys/io/Stream.hpp"

#include <stdlib.h>

namespace err {

using LL = LogLevel;

namespace {
ErrorMode error_mode;
}

ErrorMode
get_error_mode() noexcept
{
    return error_mode;
}

ErrorMode
set_error_mode(ErrorMode mode) noexcept
{
    return bl::exchange(error_mode, mode);
}

void
error(const Location *loc, LogLevel lvl, bl::string_view msg)
{
    error(loc, lvl, sys::io::stdout(), msg);
}

void
error(const Location *loc, LogLevel lvl, const char *msg)
{
    error(loc, lvl, sys::io::stdout(), bl::string_view(msg));
}

void
error(const Location *loc,
      LogLevel lvl,
      sys::io::OutStream &out,
      bl::string_view msg)
{
    reportError(out, nullptr, *loc, lvl, msg);
    if (lvl < LL::Error)
        return;

    auto mode = error_mode;
    if (lvl < LL::FatalError && mode == ErrorMode::Continue)
        return;

    switch (mode) {
    case ErrorMode::Continue:
        [[fallthrough]];
    case ErrorMode::Exit:
        exit(EXIT_FAILURE);
    case ErrorMode::Abort:
        abort();
    }
    UNREACHABLE;
}

void
error(const ErrorStaticCallSite *args)
{
    error(&args->location, args->level, args->message);
}

void
fatalError(const Location *loc, LogLevel lvl, bl::string_view msg)
{
    fatalError(loc, lvl, sys::io::stderr(), msg);
}

void
fatalError(const Location *loc, LogLevel lvl, const char *msg)
{
    fatalError(loc, lvl, sys::io::stderr(), bl::string_view(msg));
}

void
fatalError(const Location *loc,
           LogLevel lvl,
           sys::io::OutStream &out,
           bl::string_view msg)
{
    error(loc, lvl, out, msg);
    std::abort(); // keep control flow analyser happy
}

void
fatalError(const ErrorStaticCallSite *args)
{
    fatalError(&args->location, args->level, args->message);
}

void
reportError(sys::io::OutStream &out,
            const char *type,
            const Location &loc,
            LogLevel lvl,
            bl::string_view mesg)
{
    const char *prefix = type;
    bool trace = false;

    if (!prefix) {
        switch (lvl) {
        case LL::Warn:
            prefix = "WARNING";
            break;
        case LL::Assertion:
            prefix = "ASSERTION failed";
            trace = true;
            break;
        case LL::Error:
            prefix = "ERROR";
            trace = true;
            break;
        case LL::ErrorOnce:
            prefix = "ERROR (only reported once)";
            trace = true;
            break;
        case LL::FatalError:
            prefix = "FATAL ERROR";
            trace = true;
            break;
        case LL::Info:
            prefix = "INFO";
            break;
        }
    }

    out << "[" << prefix << "]"
        << " in " << loc.function << sys::io::endl
        << "  at " << loc.file << ":" << loc.line << sys::io::endl;

    if (loc.operation) {
        if (lvl == LL::Assertion)
            out << "  assertion: " << loc.operation << sys::io::endl;
        else
            out << "  operation: " << loc.operation << sys::io::endl;
    }
    out << "  message: " << mesg << sys::io::endl;

    if (HAVE_STACKTRACES_P && trace) {
        out << "  stacktrace: (most recent call first)" << sys::io::endl;
        print_stacktrace(out, "    ", 2, true);
        out << "  end of stacktrace" << sys::io::endl;
    }
}
} // namespace err
