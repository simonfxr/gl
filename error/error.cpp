#include "error/error.hpp"
#include <iostream>
#include <stdlib.h>

namespace err {

void error(const Location& loc, LogLevel lvl, const std::string& mesg) {
    error(loc, lvl, mesg.c_str());
}

void error(const Location& loc, LogLevel lvl, const char *mesg) {

    printError(std::cerr, NULL, loc, lvl, mesg);

    if (lvl == FatalError || lvl == Assertion || lvl == DebugAssertion)
        abort();
}

void fatalError(const Location& loc, const std::string& mesg) {
    fatalError(loc, mesg.c_str());
    abort(); // keep the control flow analyser happy
}

void fatalError(const Location& loc, const char *mesg) {
    error(loc, FatalError, mesg);
    abort(); // keep the control flow analyser happy
}

void printError(std::ostream& out, const char *type, const Location& loc, LogLevel lvl, const std::string& mesg) {
    printError(out, type, loc, lvl, mesg.c_str());
}

void printError(std::ostream& out, const char *type, const Location& loc, LogLevel lvl, const char *mesg) {
    const char *prefix = type;

    if (prefix == 0) {
        switch (lvl) {
        case DebugError: prefix = "ERROR (Debug)"; break;
        case DebugAssertion: prefix = "ASSERTION failed (Debug)"; break;
        case Warn: prefix = "WARNING"; break;
        case Assertion: prefix = "ASSERTION failed"; break;
        case Error: prefix = "ERROR"; break;
        case ErrorOnce: prefix = "ERROR (only reported once)"; break;
        case FatalError: prefix = "FATAL ERROR"; break;
        default: prefix = "UNKNOWN"; break;
        }
    }

    out << "[" << prefix << "]"
        << " in " << loc.function << std::endl
        << "  at " << loc.file << ":" << loc.line << std::endl;

    if (loc.operation != 0)
        out << "  operation: " << loc.operation << std::endl;

    if (lvl == DebugAssertion || lvl == Assertion)
        out << "  assertion: " << mesg << std::endl;
    else
        out << "  message: " << mesg << std::endl;
}

} // namespace err
