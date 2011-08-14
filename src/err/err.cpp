#include "err/err.hpp"
#include <iostream>
#include <stdlib.h>
#include <fstream>

namespace err {

void error(const Location& loc, std::ostream& out, LogLevel lvl, const std::string& mesg) {
    error(loc, out, lvl, mesg.c_str());
}

void error(const Location& loc, std::ostream& out, LogLevel lvl, const char *mesg) {

    printError(out, NULL, loc, lvl, mesg);

    if (lvl == FatalError || lvl == Assertion || lvl == DebugAssertion)
        abort();
}

void fatalError(const Location& loc, std::ostream& out, LogLevel lvl, const std::string& mesg) {
    fatalError(loc, out, lvl, mesg.c_str());
    abort(); // keep the control flow analyser happy
}

void fatalError(const Location& loc, std::ostream& out, LogLevel lvl, const char *mesg) {
    error(loc, out, lvl, mesg);
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
        case Info: prefix = "INFO"; break;
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

namespace {

struct LogState {
    int in_log;
    bool null;
    Location loc;
    LogLevel level;
    std::ofstream null_stream;
    std::ostream* out;

    LogState() :
        in_log(0),
        null(true),
        loc(_CURRENT_LOCATION),
        level(Info),
        null_stream(),
        out(&null_stream)
        {}
};

THREAD_LOCAL(LogState *, log_state);

void initLogState() {
    if (log_state == 0)
        log_state = new LogState;
}

bool checkLogState() {
    initLogState();
    
    if (!log_state->in_log) {
        ERR("no log begun");
        // FIXME: provide location info
        return false;
    }

    if (log_state->null)
        return false;

    return true;
}

} // namespace anon

std::ostream& logBegin(const Location& loc, const LogDestination& dest, LogLevel lvl) {
    initLogState();
    ++log_state->in_log;
    
    if (log_state->in_log > 1) {
        ERR("nested logging"); // FIXME: provide location info
        return log_state->null_stream;
    }
    
    log_state->loc = loc;
    log_state->null = lvl < dest.minimumLevel;
    log_state->out = log_state->null ? &log_state->null_stream : &dest.out;
    
    return *log_state->out;
}

std::ostream& logWrite(const std::string& msg) {
    if (checkLogState())
        *log_state->out << msg;
    return *log_state->out;
}

std::ostream& logWriteErr(const std::string& err, const std::string& msg) {
    if (checkLogState())
        error(log_state->loc, *log_state->out, log_state->level, "raised error: " + err + ", reason: " + msg);
    return *log_state->out;
}

std::ostream& logWrite(const char *msg) {
    if (checkLogState())
        *log_state->out << msg;
    return *log_state->out;
}

void logEnd() {
    if (checkLogState())
        --log_state->in_log;
    log_state->out = &log_state->null_stream;
}

void logRaiseError(const Location& loc, const LogDestination& dest, LogLevel lvl, const std::string& err, const std::string& msg) {
    if (lvl < dest.minimumLevel)
        return;
    error(loc, dest.out, lvl, "raised error: " + err + ", reason: " + msg);
}

} // namespace err
