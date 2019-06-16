#include "err/stacktrace.hpp"

#include "bl/unique_ptr.hpp"
#include "sys/io/Stream.hpp"

#include <assert.h>
#include <stdlib.h>

#if defined(ENABLE_STACKTRACES) && HU_OS_LINUX_P
#    define STACKTRACE_LINUX 1
#    include "bl/unique_ptr.hpp"
#    include <cxxabi.h>
#    include <dwarf.h>
#    include <elfutils/libdw.h>
#    include <elfutils/libdwfl.h>
#    include <execinfo.h>
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

bool
die_has_pc(Dwarf_Die *die, Dwarf_Addr pc)
{
    Dwarf_Addr low, high;

    // continuous range
    if (dwarf_hasattr(die, DW_AT_low_pc) && dwarf_hasattr(die, DW_AT_high_pc)) {
        if (dwarf_lowpc(die, &low) != 0) {
            return false;
        }
        if (dwarf_highpc(die, &high) != 0) {
            Dwarf_Attribute attr_mem;
            Dwarf_Attribute *attr = dwarf_attr(die, DW_AT_high_pc, &attr_mem);
            Dwarf_Word value;
            if (dwarf_formudata(attr, &value) != 0) {
                return false;
            }
            high = low + value;
        }
        return pc >= low && pc < high;
    }

    // non-continuous range.
    Dwarf_Addr base;
    ptrdiff_t offset = 0;
    while ((offset = dwarf_ranges(die, offset, &base, &low, &high)) > 0) {
        if (pc >= low && pc < high) {
            return true;
        }
    }
    return false;
}

Dwarf_Die *
find_fundie_by_pc(Dwarf_Die *parent_die, Dwarf_Addr pc, Dwarf_Die *result)
{
    if (dwarf_child(parent_die, result) != 0)
        return nullptr;

    Dwarf_Die *die = result;
    do {
        switch (dwarf_tag(die)) {
        case DW_TAG_subprogram:
        case DW_TAG_inlined_subroutine:
            if (die_has_pc(die, pc)) {
                return result;
            }
        };
        bool declaration = false;
        Dwarf_Attribute attr_mem;
        dwarf_formflag(dwarf_attr(die, DW_AT_declaration, &attr_mem),
                       &declaration);
        if (!declaration) {
            Dwarf_Die die_mem;
            Dwarf_Die *indie = find_fundie_by_pc(die, pc, &die_mem);
            if (indie) {
                *result = die_mem;
                return result;
            }
        }
    } while (dwarf_siblingof(die, result) == 0);

    return nullptr;
}

template<typename F>
FORCE_INLINE bool
iterate_stacktrace(size_t skip,
                   bool demangle,
                   size_t max,
                   F &&callback) noexcept
{
    char *debuginfo_path = nullptr;
    Dwfl_Callbacks callbacks;
    callbacks.find_elf = dwfl_linux_proc_find_elf;
    callbacks.find_debuginfo = dwfl_standard_find_debuginfo;
    callbacks.debuginfo_path = &debuginfo_path;

    Dwfl *dwfl = dwfl_begin(&callbacks);
    assert(dwfl != nullptr);

    auto mypid = getpid();
    auto ret = dwfl_linux_proc_report(dwfl, mypid);
    dwfl_report_end(dwfl, nullptr, nullptr);
    if (ret != 0)
        return false;

    void *trace[128];
    auto ntrace = backtrace(trace, max < 128 ? max : 128);
    using index_t = decltype(ntrace);
    if (intptr_t(ntrace) < 0)
        return false;

    for (index_t i = index_t(skip); i < ntrace; ++i) {
        FrameView nfo;
        nfo.ip = trace[i];
        auto addr = Dwarf_Addr(nfo.ip);

        auto module = dwfl_addrmodule(dwfl, addr);
        if (!module)
            return false;

        Dwarf_Addr mod_bias{};
        Dwarf_Die *cudie = dwfl_module_addrdie(module, addr, &mod_bias);
        if (!cudie) {
            while ((cudie = dwfl_module_nextcu(module, cudie, &mod_bias))) {
                Dwarf_Die die_mem;
                Dwarf_Die *fundie =
                  find_fundie_by_pc(cudie, addr - mod_bias, &die_mem);
                if (fundie) {
                    break;
                }
            }
        }

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

        auto srcloc = dwarf_getsrc_die(cudie, addr - mod_bias);
        if (srcloc) {
            nfo.file = dwarf_linesrc(srcloc, nullptr, nullptr);
            dwarf_lineno(srcloc, &nfo.line);
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
#    if HU_BITS_32_P
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
