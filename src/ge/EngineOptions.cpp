#include "ge/EngineOptions.hpp"
#include "glt/utils.hpp"
#include "opengl.hpp"

#include "sys/fs.hpp"
#include "sys/io/Stream.hpp"

#include <cctype>
#include <cstring>

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
    GlewExp,
    DisableRender
};

struct Option
{
    const char *option;
    const char *option_arg;
    OptionCase option_case;
    const char *description;
};

const Option OPTIONS[] = {
    { "--help", nullptr, Help, "print a description of all options and exit" },
    { "--no-init-script",
      nullptr,
      NoInitScript,
      "inhibit loading of default script file (<program-name>.script)" },
    { "--script-dir",
      "DIR",
      ScriptDir,
      "add DIR to the list of searched script directories" },
    { "--shader-dir",
      "DIR",
      ShaderDir,
      "add DIR to the list of searched shader directories" },
    { "--cd",
      "BOOL",
      CD,
      "change into the base directory of the program binary: yes|no" },
    { "--eval", "COMMAND", Eval, "execute COMMAND as a script command" },
    { "--script", "FILE", Script, "load FILE and execute it as a script" },
    { "--cwd", "DIR", CWD, "change into directory DIR" },
    { "--gl-version",
      "VERSION",
      GLVersion,
      "set the OpenGL context version: MAJOR.MINOR" },
    { "--gl-profile",
      "TYPE",
      GLProfile,
      "set the OpenGL profile type: core|compatibility" },
    { "--gl-debug", "BOOL", GLDebug, "create a OpenGL debug context: yes|no" },
    { "--gl-trace", "BOOL", GLTrace, "print every OpenGL call: yes|no" },
    { "--aa-samples", "NUM", AASamples, "set the number of FSAA Samples" },
    { "--vsync", "BOOL", VSync, "enable vertical sync: yes|no" },
    { "--glew-experimental",
      "BOOL",
      GlewExp,
      "set the glewExperimental flag: yes|no" },
    { "--disable-render",
      "BOOL",
      DisableRender,
      "disable calling the renderfunction" }
};

struct State
{
    EngineOptions &options;

    explicit State(EngineOptions &opts) : options(opts) {}

    bool option(OptionCase opt, const char *arg);
};

#define CMDWARN(msg) WARN(std::string("parsing options: ") + (msg))

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
        options.scriptDirs.emplace_back(arg);
        return true;
    case ShaderDir:
        options.shaderDirs.emplace_back(arg);
        return true;
    case CD:
        if (!parse_bool(arg, options.defaultCD)) {
            CMDWARN("--cd: not a boolean option");
            return false;
        }
        return true;
    case Eval:
        options.commands.emplace_back(EngineOptions::Command, std::string(arg));
        return true;
    case Script:
        options.commands.emplace_back(EngineOptions::Script, std::string(arg));
        return true;
    case CWD:
        options.workingDirectory = arg;
        return true;
    case GLVersion:
        int maj, min;
        if (sscanf(arg, "%d.%d", &maj, &min) != 2 || maj < 0 || min < 0 ||
            maj > 9 || min > 9) {
            CMDWARN("invalid OpenGL Version: " + std::string(arg));
            return false;
        }
        options.window.settings.majorVersion = unsigned(maj);
        options.window.settings.minorVersion = unsigned(min);
        return true;

    case GLProfile:
        if (str_eq(arg, "core")) {
            options.window.settings.coreProfile = true;
        } else if (str_eq(arg, "compat") || str_eq(arg, "compatibility")) {
            options.window.settings.coreProfile = false;
        } else {
            CMDWARN("--gl-profile: invalid OpenGL Profile type: " +
                    std::string(arg));
            return false;
        }
        return true;
    case GLDebug:
        if (!parse_bool(arg, options.window.settings.debugContext)) {
            CMDWARN("--gl-debug: not a boolean option");
            return false;
        }
        return true;
    case GLTrace:
        if (!parse_bool(arg, options.traceOpenGL)) {
            CMDWARN("--gl-trace: not a boolean option");
            return false;
        }
        return true;
    case AASamples:
        unsigned samples;
        if (sscanf(arg, "%u", &samples) != 1) {
            CMDWARN("--aa-samples: not an integer");
            return false;
        }

        options.window.settings.antialiasingLevel = samples;
        return true;
    case VSync: {
        if (!parse_bool(arg, options.window.vsync)) {
            CMDWARN("--vsync: not a boolean option");
            return false;
        }
        return true;
    }
    case GlewExp:
        bool yesno;
        if (!parse_bool(arg, yesno)) {
            CMDWARN("--gl-debug: not a boolean option");
            return false;
        }
        glewExperimental = glt::gl_bool(yesno);
        return true;
    case DisableRender:
        if (!parse_bool(arg, options.disableRender)) {
            CMDWARN("--disable-render: not a boolean option");
            return false;
        }
        return true;
        // default:
        //     FATAL_ERR("unhandled option_case");
        //     return false;
    }
}

} // namespace

EngineOptions::EngineOptions()
  : workingDirectory(SOURCE_DIR)
  , inhibitInitScript(false)
  , defaultCD(false)
  , mode(Animate)
  , traceOpenGL(false)
  , disableRender(false)
{
    window.settings.majorVersion = 3;
    window.settings.minorVersion = 3;
#ifdef GLDEBUG
    window.settings.debugContext = true;
#endif
}

EngineOptions &
EngineOptions::parse(int *argcp, char ***argvp)
{

    char **argv = *argvp;
    int argc = *argcp;

    int dest = 1;

    State state(*this);
    if (argc > 0) {
        binary = sys::fs::absolutePath(argv[0]);
        window.title = sys::fs::basename(binary);
    }

    bool done = false;
    int i;
    for (i = 1; i < argc && !done;) {
        bool err = false;
        int skip = 0;

        if (argv[i] != nullptr && argv[i][0] == '-') {

            if (str_eq(argv[i], "--")) {
                skip = 1;
                done = true;
                goto skip_args;
            }

            for (const auto &opt : OPTIONS) {
                if (str_eq(argv[i], opt.option)) {
                    bool has_arg = opt.option_arg != nullptr;

                    if (!has_arg) {
                        skip = 1;
                        err = !state.option(opt.option_case, nullptr);
                    } else if (i + 1 >= argc) {
                        skip = 1;
                        CMDWARN(std::string(opt.option) + " missing argument");
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

    out << "Engine options: " << sys::io::endl;

    int max_col = 0;

    for (const auto &i : OPTIONS) {
        defs::index_t w = 2;
        w += SIZE(strlen(i.option));
        if (i.option_arg != nullptr)
            w += 1 + SIZE(strlen(i.option_arg));
        if (w > max_col)
            max_col = w;
    }

    for (const auto &i : OPTIONS) {
        defs::index_t w = 2;
        out << "  " << i.option;
        w += SIZE(strlen(i.option));
        if (i.option_arg != nullptr) {
            out << " " << i.option_arg;
            w += 1 + SIZE(strlen(i.option_arg));
        }

        while (w++ < max_col + 3)
            out << ' ';

        out << i.description << sys::io::endl;
    }

    out << sys::io::endl;
}

} // namespace ge
