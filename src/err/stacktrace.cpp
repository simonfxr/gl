#include "err/stacktrace.hpp"

#include "bl/unique_ptr.hpp"
#include "sys/io/Stream.hpp"

#include <assert.h>
#include <stdlib.h>

#if defined(ENABLE_STACKTRACES) && HU_OS_LINUX_P
#    define STACKTRACE_LINUX 1
#    define UNW_LOCAL_ONLY
#    include "bl/unique_ptr.hpp"
#    include <cxxabi.h>
#    include <elfutils/libdwfl.h>
#    include <libunwind.h>
#    include <unistd.h>
#elif defined(ENABLE_STACKTRACES) && defined(HU_OS_WINDOWS)
#    define STACKTRACE_WINDOWS
#    include <DbgHelp.h>
#    include <Windows.h>
#elif defined(ENABLE_STACKTRACES) && __has_include(<execinfo.h>)
#    define STACKTRACE_EXECINFO
#    include <cxxabi.h>
#    include <execinfo.h>
#endif

namespace err {

namespace {

struct FrameView
{
    const char *function{};
    char *demangled_function{};
    const char *file{};
    int line{};
    void *ip{};
    const char *module{};

    FrameView() = default;
    FrameView(const FrameView &) = delete;
    FrameView(FrameView &&) = delete;

    FrameView &operator=(const FrameView &) = delete;
    FrameView &operator=(FrameView &&) = delete;

    ~FrameView()
    {
#if HU_OS_POSIX_P
        free(demangled_function);
#endif
    }

    FrameInfo alloc_info()
    {
        return { bl::string(function),
                 bl::string(demangled_function),
                 bl::string(file),
                 line,
                 ip,
                 bl::string(module) };
    }
};

#ifdef STACKTRACE_LINUX

template<typename F>
FORCE_INLINE bool
iterate_stacktrace(size_t skip,
                   bool demangle,
                   size_t max,
                   F &&callback) noexcept
{
    unw_context_t uc;
    unw_getcontext(&uc);

    unw_cursor_t cursor;
    unw_init_local(&cursor, &uc);

    char *debuginfo_path = nullptr;
    Dwfl_Callbacks callbacks;
    callbacks.find_elf = dwfl_linux_proc_find_elf;
    callbacks.find_debuginfo = dwfl_standard_find_debuginfo;
    callbacks.debuginfo_path = &debuginfo_path;

    Dwfl *dwfl = dwfl_begin(&callbacks);
    assert(dwfl != nullptr);

    dwfl_report_begin(dwfl);
    auto ret = dwfl_linux_proc_report(dwfl, getpid());
    dwfl_report_end(dwfl, nullptr, nullptr);
    if (ret != 0)
        return false;

    while (unw_step(&cursor) > 0) {
        unw_word_t unw_ip;
        unw_get_reg(&cursor, UNW_REG_IP, &unw_ip);

        if (skip > 0) {
            --skip;
            continue;
        }

        if (max-- == 0)
            break;

        FrameView nfo;
        nfo.ip = reinterpret_cast<void **>(unw_ip) - 1;
        auto addr = Dwarf_Addr(nfo.ip);
        Dwfl_Module *module = dwfl_addrmodule(dwfl, addr);

        nfo.function = dwfl_module_addrname(module, addr);
        if (demangle && nfo.function) {
            int status = 0;
            size_t len = 0;
            nfo.demangled_function =
              abi::__cxa_demangle(nfo.function, nullptr, &len, &status);
        }

        nfo.module = dwfl_module_info(module,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      nullptr);

        Dwfl_Line *line = dwfl_getsrc(dwfl, addr);
        if (line != nullptr) {
            Dwarf_Addr faddr;
            nfo.file =
              dwfl_lineinfo(line, &faddr, &nfo.line, nullptr, nullptr, nullptr);
        }

        callback(nfo);
    }

    return true;
}

#elif defined(STACKTRACE_WINDOWS)

struct ProcessContext
{
    HANDLE process;
    ProcessContext()
    {
        process = GetCurrentProcess();
        SymInitialize(process, nullptr, TRUE);
        SymSetOptions(SYMOPT_LOAD_LINES);
    }
};

void
print_stacktrace(sys::io::OutStream &out, int skip)
{
    static ProcessContext proc_context;
    auto process = proc_context.process;

    HANDLE thread = GetCurrentThread();
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

    STACKFRAME frame = {};
#    ifdef HU_BITS_32
    DWORD machine = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset = context.Eip;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrStack.Offset = context.Esp;
    const auto tableAccess = SymFunctionTableAccess;
    const auto getModuleBase = SymGetModuleBase;
    const auto walk = StackWalk;
#    else
    DWORD machine = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset = context.Rip;
    frame.AddrFrame.Offset = context.Rbp;
    frame.AddrStack.Offset = context.Rsp;
    const auto tableAccess = SymFunctionTableAccess64;
    const auto getModuleBase = SymGetModuleBase64;
    const auto walk = StackWalk64;
#    endif
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;

    while (walk(machine,
                process,
                thread,
                &frame,
                &context,
                nullptr,
                tableAccess,
                getModuleBase,
                nullptr)) {

        if (skip > 0) {
            --skip;
            continue;
        }

        // auto functionAddress = frame.AddrPC.Offset;

        // auto moduleBase = SymGetModuleBase(process, frame.AddrPC.Offset);
        // bool have_module = false;
        // if (moduleBase &&
        //    GetModuleFileNameA((HINSTANCE) moduleBase, moduleBuff, MAX_PATH))
        //    { have_module = true;
        // }

        char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255];
        PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL) symbolBuffer;
        symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL) + 255;
        symbol->MaxNameLength = 254;

        out << " ";
        if (SymGetSymFromAddr(process, frame.AddrPC.Offset, nullptr, symbol)) {
            DWORD offset = 0;
            IMAGEHLP_LINE line;
            line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

            out << symbol->Name << "()";

            if (SymGetLineFromAddr(
                  process, frame.AddrPC.Offset, &offset, &line)) {
                out << "[" << line.FileName << ":" << line.LineNumber << "]";
            }
        } else {
            char buf[20];
            snprintf(buf,
                     sizeof buf,
                     "%p",
                     reinterpret_cast<void *>(frame.AddrPC.Offset));
            out << "ip:" << buf;
        }
        // if (have_module)
        //    out << " in " << moduleBuff;
        out << sys::io::endl;
    }
}
#elif defined(STACKTRACE_EXECINFO)

struct DeleterFree
{
    void operator()(char **p) const noexcept { free(p); }
};

template<typename F>
bool
iterate_stacktrace(size_t skip, bool demangle, size_t max, F &&callback)
{
    bl::vector<void *> trace;
    if (max > 256)
        max = 256;
    trace.resize(max);
    auto n = backtrace(trace.data(), trace.size());
    // on linux index_t = int, but on BSD index_t = size_t
    using index_t = decltype(n);
    bl::unique_ptr<char *, DeleterFree> syms;
    syms.reset(backtrace_symbols(trace.data(), n));

    if (size_t(index_t(skip)) != skip)
        return false;

    for (auto i = index_t(skip); i < n; ++i) {
        FrameView nfo;
        nfo.ip = trace[i];
        nfo.function = syms.get()[i];
        if (demangle && nfo.function) {
            int status = 0;
            size_t len = 0;
            nfo.demangled_function =
              abi::__cxa_demangle(nfo.function, nullptr, &len, &status);
        }

        callback(nfo);
    }

    return true;
}

#else

template<typename F>
bool
iterate_stacktrace(size_t, bool, size_t, F &&)
{
    return false;
}

#endif
} // namespace

bl::array_view<FrameInfo>
stacktrace(bl::array_view<FrameInfo> frames,
           size_t skip,
           bool demangle) noexcept
{

    if (frames.empty())
        return frames;

    size_t idx = 0;
    iterate_stacktrace(
      skip, demangle, frames.size(), [&idx, &frames](FrameView &nfo) {
          frames[idx++] = nfo.alloc_info();
      });

    return frames.subspan(0, idx);
}

bl::vector<FrameInfo>
stacktrace(size_t skip, bool demangle, size_t max) noexcept
{
    bl::vector<FrameInfo> frames;
    iterate_stacktrace(skip, demangle, max, [&frames](FrameView &nfo) {
        frames.emplace_back(nfo.alloc_info());
    });
    return frames;
}

void
print_stacktrace(sys::io::OutStream &out,
                 bl::string_view line_prefix,
                 size_t skip,
                 bool demangle,
                 size_t max) noexcept
{
    auto ret = iterate_stacktrace(
      skip, demangle, max, [&out, line_prefix](const FrameView &nfo) {
          out << line_prefix;
          if (nfo.demangled_function) {
              out << nfo.demangled_function;
          } else if (nfo.function) {
              out << nfo.function << "()";
          }

          out << "[";
          if (nfo.file)
              out << nfo.file << ":" << nfo.line << " ";
          out << "ip:" << nfo.ip << "]";
          out << sys::io::endl;
      });
    if (!HAVE_STACKTRACES_P) {
        out << "ERROR: stacktraces are disabled" << sys::io::endl;
    } else if (!ret) {
        out << "ERROR: getting stacktrace failed" << sys::io::endl;
    }
}

} // namespace err
