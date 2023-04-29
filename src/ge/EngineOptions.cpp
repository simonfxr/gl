#ifdef _WIN32
#    define _CRT_SECURE_NO_WARNINGS 1
#endif

#include "ge/EngineOptions.hpp"

#include "glt/utils.hpp"
#include "opengl.hpp"
#include "pp/basic.h"
#include "sys/fs.hpp"
#include "sys/io/Stream.hpp"
#include "util/string.hpp"

#include <cctype>
#include <cstring>

using namespace bl::literals;

namespace ge {

namespace {

enum OptionCase
{
    Help,
    NoInitScript,
    ScriptDir,
    ShaderDir,
    CD,
    Eval,
    Script,
    CWD,
    GLVersion,
    GLProfile,
    GLDebug,
    GLTrace,
    AASamples,
    VSync,
    DisableRender,
    DumpShaders
};

struct Option
{
    bl::string_view option;
    bl::string_view option_arg;
    OptionCase option_case;
    bl::string_view description;
};

// clang-format off
const constexpr auto OPTIONS = std::to_array<Option>({
  { "--help"_k, ""_k, Help, "print a description of all options and exit"_k },
  { "--no-init-script"_k, ""_k, NoInitScript, "inhibit loading of default script file (<program-name>.script)"_k },
  { "--script-dir"_k, "DIR"_k, ScriptDir, "add DIR to the list of searched script directories"_k },
  { "--shader-dir"_k, "DIR"_k, ShaderDir, "add DIR to the list of searched shader directories"_k },
  { "--cd"_k, "BOOL"_k, CD, "change into the base directory of the program binary: yes|no"_k },
  { "--eval"_k, "COMMAND"_k, Eval, "execute COMMAND as a script command"_k },
  { "--script"_k, "FILE"_k, Script, "load FILE and execute it as a script"_k },
  { "--cwd"_k, "DIR"_k, CWD, "change into directory DIR"_k },
  { "--gl-version"_k, "VERSION"_k, GLVersion, "set the OpenGL context version: MAJOR.MINOR"_k },
  { "--gl-profile"_k, "TYPE"_k, GLProfile, "set the OpenGL profile type: core|compatibility"_k },
  { "--gl-debug"_k, "BOOL"_k, GLDebug, "create a OpenGL debug context: yes|no"_k },
  { "--gl-trace"_k, "BOOL"_k, GLTrace, "print every OpenGL call: yes|no"_k },
  { "--aa-samples"_k, "NUM"_k, AASamples, "set the number of FSAA Samples"_k },
  { "--vsync"_k, "BOOL"_k, VSync, "enable vertical sync: yes|no"_k },
  { "--disable-render"_k, "BOOL"_k, DisableRender, "disable calling the renderfunction"_k },
  { "--dump-shaders"_k, "BOOL"_k, DumpShaders, "dump shader source after preprocessing"_k },
});
// clang-format on

struct State
{
    EngineOptions &options;

    explicit State(EngineOptions &opts) : options(opts) {}

    bool option(OptionCase opt, const char *arg);
};

#define CMDWARN(...) WARN(scat("parsing options: "_k, __VA_ARGS__))

bool
str_eq(const char *a, const char *b)
{
    return strcmp(a ? a : "", b ? b : "") == 0;
}

bool
str_eqi(const char *a, const char *b)
{
    if (a == b)
        return true;
    if (a == nullptr || b == nullptr)
        return false;

    while (tolower(*a) == tolower(*b++))
        if (*a++ == '\0')
            return false;

    return true;
}

bool
parse_bool(const char *arg, bool &dest)
{
    if (str_eqi(arg, "yes") || str_eqi(arg, "true"))
        dest = true;
    else if (str_eqi(arg, "no") || str_eqi(arg, "false"))
        dest = false;
    else
        return false;
    return true;
}

bool
State::option(OptionCase opt, const char *arg)
{
    switch (OPTIONS[opt].option_case) {
    case Help:
        options.mode = EngineOptions::Help;
        return true;
    case NoInitScript:
        options.inhibitInitScript = true;
        return true;
    case ScriptDir:
        options.scriptDirs.emplace_back(bl::from_cstring(arg));
        return true;
    case ShaderDir:
        options.shaderDirs.emplace_back(bl::from_cstring(arg));
        return true;
    case CD:
        if (!parse_bool(arg, options.defaultCD)) {
            CMDWARN("--cd: not a boolean option"_k);
            return false;
        }
        return true;
    case Eval:
        options.commands.emplace_back(EngineOptions::Command,
                                      bl::from_cstring(arg));
        return true;
    case Script:
        options.commands.emplace_back(EngineOptions::Script,
                                      bl::from_cstring(arg));
        return true;
    case CWD:
        options.workingDirectory = bl::from_cstring(arg);
        return true;
    case GLVersion:
        int maj, min;
        if (sscanf(arg, "%d.%d", &maj, &min) != 2 || maj < 0 || min < 0 ||
            maj > 9 || min > 9) {
            CMDWARN("invalid OpenGL Version: "_k, bl::from_cstring(arg));
            return false;
        }
        options.window.settings.majorVersion = static_cast<unsigned>(maj);
        options.window.settings.minorVersion = static_cast<unsigned>(min);
        return true;

    case GLProfile:
        if (str_eq(arg, "core")) {
            options.window.settings.coreProfile = true;
        } else if (str_eq(arg, "compat") || str_eq(arg, "compatibility")) {
            options.window.settings.coreProfile = false;
        } else {
            CMDWARN("--gl-profile: invalid OpenGL Profile type: "_k,
                    bl::from_cstring(arg));
            return false;
        }
        return true;
    case GLDebug:
        if (!parse_bool(arg, options.window.settings.debugContext)) {
            CMDWARN("--gl-debug: not a boolean option"_k);
            return false;
        }
        return true;
    case GLTrace:
        if (!parse_bool(arg, options.traceOpenGL)) {
            CMDWARN("--gl-trace: not a boolean option"_k);
            return false;
        }
        return true;
    case AASamples:
        unsigned samples;
        if (sscanf(arg, "%u", &samples) != 1) {
            CMDWARN("--aa-samples: not an integer"_k);
            return false;
        }

        options.window.settings.antialiasingLevel = samples;
        return true;
    case VSync: {
        if (!parse_bool(arg, options.window.vsync)) {
            CMDWARN("--vsync: not a boolean option"_k);
            return false;
        }
        return true;
    }
    case DisableRender:
        if (!parse_bool(arg, options.disableRender)) {
            CMDWARN("--disable-render: not a boolean option"_k);
            return false;
        }
        return true;
    case DumpShaders:
        if (!parse_bool(arg, options.dumpShaders)) {
            CMDWARN("--dump-shaders: not a boolean option"_k);
            return false;
        }
        return true;
    }
    FATAL_ERR("foo");
}

} // namespace

EngineOptions::EngineOptions()
  : workingDirectory(bl::string_view_literal(PP_TOSTR(SOURCE_DIR)))
  , inhibitInitScript(false)
  , defaultCD(false)
  , mode(Animate)
  , traceOpenGL(false)
  , disableRender(false)
  , dumpShaders(false)
{
    window.settings.majorVersion = 3;
    window.settings.minorVersion = 3;
#if ENABLE_GLDEBUG_P
    window.settings.debugContext = true;
#endif
}

EngineOptions &
EngineOptions::parse(int *argcp, char ***argvp)
{

    auto argv = *argvp;
    auto argc = *argcp;

    auto dest = 1;

    auto str_view = [](auto s) {
        if (s)
            return bl::string_view{ s, strlen(s) };
        return ""_k;
    };

    auto state = State{ *this };
    if (argc > 0) {
        binary = sys::fs::absolutePath(str_view(argv[0]));
        window.title = sys::fs::dropExtension(sys::fs::basename(binary));
    }

    auto done = false;
    int i;
    for (i = 1; i < argc && !done;) {
        auto arg = str_view(argv[i]);
        auto err = false;
        auto skip = 0;

        if (arg == "-"_k) {

            if (arg == "--"_k) {
                skip = 1;
                done = true;
                goto skip_args;
            }

            for (const auto &opt : OPTIONS) {
                if (arg == opt.option) {
                    auto has_arg = !opt.option_arg.empty();

                    if (!has_arg) {
                        skip = 1;
                        err = !state.option(opt.option_case, nullptr);
                    } else if (i + 1 >= argc) {
                        skip = 1;
                        CMDWARN(opt.option, " missing argument"_k);
                    } else {
                        skip = 2;
                        err = !state.option(opt.option_case, argv[i + 1]);
                    }
                }
            }
        }

    skip_args:;

        if (skip == 0) {
            argv[dest++] = argv[i];
            ++i;
        }

        i += skip;

        UNUSED(err);
    }

    while (i < argc)
        argv[dest++] = argv[i++];

    argv[dest] = nullptr;
    *argcp = dest;
    return *this;
}

void
EngineOptions::printHelp()
{
    sys::io::OutStream &out = sys::io::stderr();

    out << "Engine options:\n"_k;

    auto max_col = size_t{ 0 };

    for (const auto &i : OPTIONS) {
        auto w = size_t{ 2 };
        w += i.option.size();
        if (!i.option_arg.empty())
            w += 1 + i.option_arg.size();
        if (w > max_col)
            max_col = w;
    }

    for (const auto &i : OPTIONS) {
        auto w = size_t{ 2 };
        out << "  "_k << i.option;
        w += i.option.size();
        if (!i.option_arg.empty()) {
            out << " "_k << i.option_arg;
            w += 1 + i.option_arg.size();
        }

        while (w++ < max_col + 3)
            out << ' ';

        out << i.description << "\n"_k;
    }

    out << "\n"_k;
}

} // namespace ge
