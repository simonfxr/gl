#include "err/err.hpp"
#include <stdlib.h>

#ifdef UNWIND_STACKTRACES
#define UNW_LOCAL_ONLY

#include <elfutils/libdwfl.h>
#include <libunwind.h>
#include <assert.h>
#include <unistd.h>
#include <cxxabi.h>

#endif

extern "C" void __clang_trap_function() {}

namespace err {

namespace {

void print_stacktrace(sys::io::OutStream&, int skip = 0) ATTRS(ATTR_NOINLINE);

#ifdef UNWIND_STACKTRACES

void debugInfo(sys::io::OutStream& out, const void *ip) {

    char *debuginfo_path = 0;

    Dwfl_Callbacks callbacks;
    callbacks.find_elf = dwfl_linux_proc_find_elf;
    callbacks.find_debuginfo = dwfl_standard_find_debuginfo;
    callbacks.debuginfo_path = &debuginfo_path;

    Dwfl *dwfl = dwfl_begin(&callbacks);
    assert(dwfl!=NULL);

    assert(dwfl_linux_proc_report (dwfl, getpid())==0);
    assert(dwfl_report_end (dwfl, NULL, NULL)==0);

    Dwarf_Addr addr = Dwarf_Addr(ip);

    Dwfl_Module *module = dwfl_addrmodule (dwfl, addr);
    
    const char *function_name = dwfl_module_addrname(module, addr);
    int status = 0;
    char *demangled = abi::__cxa_demangle(function_name, 0, 0, &status);
        
    out << (demangled ? demangled : function_name) << "[";
    if (demangled) free(demangled);

    Dwfl_Line *line = 0;
    for (int i = -0xF; i <= 0xF; ++i) {
        
        line = dwfl_getsrc(dwfl, Dwarf_Addr((const char *) addr + i));
        if (line != 0)
            break;
    }

    if (line != 0) {
        int nline;
        Dwarf_Addr addr;
        const char *filename = dwfl_lineinfo(line, &addr, &nline, 0, 0, 0);
        out << filename << ":" << nline;
    } else {
        out << "ip: " << ip;
    }

    out << ']';
}

void print_stacktrace(sys::io::OutStream& out, int skip) {
    unw_context_t uc;
    unw_getcontext(&uc);

    unw_cursor_t cursor;
    unw_init_local(&cursor, &uc);

    out << "  stacktrace: (most recent call first)" << sys::io::endl;

    while (unw_step(&cursor) > 0) {
        unw_word_t ip;
        unw_get_reg(&cursor, UNW_REG_IP, &ip);

        // unw_word_t offset;
        // char name[256];
        // unw_get_proc_name(&cursor, name,sizeof(name), &offset);

        if (skip <= 0) {
            out << "    ";
            debugInfo(out, (void *)((void **) ip - 1));
            out << sys::io::endl;
        } else {
            skip--;
        }
    }

    out << "  end of stacktrace" << sys::io::endl;
}

#else

void print_stacktrace(sys::io::OutStream& out, int) {
    out << "  stacktrace: not available" << sys::io::endl;
}

#endif

} // namespace anon

void error(const Location& loc, LogLevel lvl, const ErrorArgs& args) {
    printError(args.out, 0, loc, lvl, args.mesg);

    if (lvl == FatalError || lvl == Assertion || lvl == DebugAssertion)
        abort();
}

void fatalError(const Location& loc, LogLevel lvl, const ErrorArgs& args) {
    error(loc, lvl, args);
    abort(); // keep the control flow analyser happy
}

void printError(sys::io::OutStream& out, const char *type, const Location& loc, LogLevel lvl, const char *mesg) {
    const char *prefix = type;
    bool st = false;

    if (prefix == 0) {
        switch (lvl) {
        case DebugError: prefix = "ERROR (Debug)"; st = true; break;
        case DebugAssertion: prefix = "ASSERTION failed (Debug)"; st = true; break;
        case Warn: prefix = "WARNING"; break;
        case Assertion: prefix = "ASSERTION failed"; st = true; break;
        case Error: prefix = "ERROR"; st = true; break;
        case ErrorOnce: prefix = "ERROR (only reported once)"; st = true; break;
        case FatalError: prefix = "FATAL ERROR"; st = true; break;
        case Info: prefix = "INFO"; break;
        default: prefix = "UNKNOWN"; st = true; break;
        }
    }

    out << "[" << prefix << "]"
        << " in " << loc.function << sys::io::endl
        << "  at " << loc.file << ":" << loc.line << sys::io::endl;


    if (loc.operation != 0)
        out << "  operation: " << loc.operation << sys::io::endl;

    if (lvl == DebugAssertion || lvl == Assertion)
        out << "  assertion: " << mesg << sys::io::endl;
    else
        out << "  message: " << mesg << sys::io::endl;
    
    if (st)
        print_stacktrace(out);
}

namespace {

struct LogState {
    int in_log;
    bool null;
    Location loc;
    LogLevel level;
    sys::io::NullStream null_stream;
    sys::io::OutStream* out;

    LogState() :
        in_log(0),
        null(true),
        loc(_CURRENT_LOCATION),
        level(Info),
        out(&null_stream)
        {}
    
private:
    LogState(const LogState&);
    LogState& operator =(const LogState&);
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

sys::io::OutStream& logBegin(const Location& loc, const LogDestination& dest, LogLevel lvl) {
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

sys::io::OutStream& logWrite(const std::string& msg) {
    if (checkLogState())
        *log_state->out << msg;
    return *log_state->out;
}

sys::io::OutStream& logWriteErr(const std::string& err, const std::string& msg) {
    if (checkLogState())
        error(log_state->loc, log_state->level, ErrorArgs(*log_state->out, "raised error: " + err + ", reason: " + msg));
    return *log_state->out;
}

sys::io::OutStream& logWrite(const char *msg) {
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
    error(loc, lvl, ErrorArgs(dest.out, "raised error: " + err + ", reason: " + msg));
}

} // namespace err
