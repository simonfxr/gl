#include "opengl.hpp"
#include "ge/EngineOptions.hpp"

#include "sys/fs/fs.hpp"

#include <string.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

namespace ge {

namespace {

enum OptionCase {
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
    GlewExp
};

struct Option {
    const char *option;
    const char *option_arg;
    OptionCase option_case;
    const char *description;
};

const Option OPTIONS[] = {
    { "--help", NULL, Help, "print a description of all options and exit" },
    { "--no-init-script", NULL, NoInitScript, "inhibit loading of default script file (<program-name>.script)" },
    { "--script-dir", "DIR", ScriptDir, "add DIR to the list of searched script directories" },
    { "--shader-dir", "DIR", ShaderDir, "add DIR to the list of searched shader directories" },
    { "--cd", "BOOL", CD, "dchange into the base directory of the program binary: yes|no" },
    { "--eval", "COMMAND", Eval, "execute COMMAND as a script command" },
    { "--script", "FILE", Script, "load FILE and execute it as a script" },
    { "--cwd", "DIR", CWD, "change into directory DIR" },
    { "--gl-version", "VERSION", GLVersion, "set the OpenGL context version: MAJOR.MINOR" },
    { "--gl-profile", "TYPE", GLProfile, "set the OpenGL profile type: core|compatibility" },
    { "--gl-debug", "BOOL", GLDebug, "create a OpenGL debug context: yes|no" },
    { "--gl-trace", "BOOL", GLTrace, "print every OpenGL call: yes|no" },
    { "--aa-samples", "NUM", AASamples, "set the number of FSAA Samples" },
    { "--vsync", "BOOL", VSync, "enable vertical sync: yes|no" },
    { "--glew-experimental", "BOOL", GlewExp, "set the glewExperimental flag: yes|no" }
};

struct State {
    EngineOptions& options;
    
    State(EngineOptions& opts) :
        options(opts)
        {}

    bool option(OptionCase opt, const char *arg);
};

#define CMDWARN(msg) WARN(std::string("parsing options: ") + (msg))

bool str_eq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

bool str_eqi(const char *a, const char *b) {
    if (a == b) return true;
    if (a == 0 || b == 0) return false;
    
    while (tolower(*a) == tolower(*b++))
        if (*a++ == '\0')
            return false;

    return true;
}

bool parse_bool(const char *arg, bool& dest) {
    if (str_eqi(arg, "yes") || str_eqi(arg, "true"))
        dest = true;
    else if (str_eqi(arg, "no") || str_eqi(arg, "false"))
        dest = false;
    else
        return false;
    return true;
}

bool State::option(OptionCase opt, const char *arg) {
    switch (OPTIONS[opt].option_case) {
    case Help:
        options.mode = EngineOptions::Help;
        return true;
    case NoInitScript:
        options.inhibitInitScript = true;
        return true;
    case ScriptDir:
        options.scriptDirs.push_back(arg);
        return true;
    case ShaderDir:
        options.shaderDirs.push_back(arg);
        return true;
    case CD:
        if (!parse_bool(arg, options.defaultCD)) {
            CMDWARN("--cd: not a boolean option");
            return false;
        }
        return true;
    case Eval:
        options.commands.push_back(std::make_pair(EngineOptions::Command, std::string(arg)));
        return true;
    case Script:
        options.commands.push_back(std::make_pair(EngineOptions::Script, std::string(arg)));
        return true;
    case CWD:
        options.workingDirectory = arg;
        return true;
    case GLVersion:
        int maj, min;
        if (sscanf(arg, "%d.%d", &maj, &min) != 2 || maj < 0 || min < 0 || maj > 9 || min > 9) {
            CMDWARN("invalid OpenGL Version: " + std::string(arg));
            return false;
        } else {
            options.window.settings.MajorVersion = unsigned(maj);
            options.window.settings.MinorVersion = unsigned(min);
            return true;
        }
    case GLProfile:
        if (str_eq(arg, "core")) {
            options.window.settings.CoreProfile = true;
        } else if (str_eq(arg, "compat") || str_eq(arg, "compatibility")) {
            options.window.settings.CoreProfile = false;
        } else {
            CMDWARN("--gl-profile: invalid OpenGL Profile type: " + std::string(arg));
            return false;
        }
        return true;
    case GLDebug:
        if (!parse_bool(arg, options.window.settings.DebugContext)) {
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
        
        options.window.settings.AntialiasingLevel = samples;
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
        glewExperimental = yesno ? GL_TRUE : GL_FALSE;
        return true;
    default:
        FATAL_ERR("unhandled option_case");
        return false;
    }
}

} // namespace anon

EngineOptions::EngineOptions() :
    commands(),
    workingDirectory(),
    inhibitInitScript(false),
    defaultCD(false),
    shaderDirs(),
    scriptDirs(),
    window(),
    mode(Animate),
    traceOpenGL(false)
{
#ifdef GLDEBUG
    window.settings.DebugContext = true;
#endif
    scriptDirs.push_back("scripts");
}

EngineOptions& EngineOptions::parse(int *argcp, char ***argvp) {

    char **argv = *argvp;
    int argc = *argcp;

    int dest = 1;

    State state(*this);
    if (argc > 0)
        binary = sys::fs::absolutePath(argv[0]);

    bool done = false;
    int i;
    for (i = 1; i < argc && !done; ) {
        bool err = false;
        int skip = 0;
        

        if (argv[i] != 0 && argv[i][0] == '-') {

            if (str_eq(argv[i], "--")) {
                skip = 1;
                done = true;
                goto skip_args;
            }

            for (defs::index opt = 0; opt < ARRAY_LENGTH(OPTIONS); ++opt) {
                if (str_eq(argv[i], OPTIONS[opt].option)) {
                    bool has_arg = OPTIONS[opt].option_arg != 0;

                    if (!has_arg) {
                        skip = 1;
                        err = !state.option(OPTIONS[opt].option_case, NULL);
                    } else if (i + 1 >= argc) {
                        skip = 1;
                        CMDWARN(std::string(OPTIONS[opt].option) + " missing argument");
                    } else {
                        skip = 2;
                        err = !state.option(OPTIONS[opt].option_case, argv[i + 1]);
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

    argv[dest] = 0;
    *argcp = dest;
    return *this;
}

void EngineOptions::printHelp() {
    std::ostream& out = std::cerr;

    out << "Engine options: " << std::endl;

    int max_col = 0;

    for (defs::index i = 0; i < ARRAY_LENGTH(OPTIONS); ++i) {
        defs::index w = 2;
        w += SIZE(strlen(OPTIONS[i].option));
        if (OPTIONS[i].option_arg != 0)
            w += 1 + SIZE(strlen(OPTIONS[i].option_arg));
        if (w > max_col)
            max_col = w;
    }

    for (defs::index i = 0; i < ARRAY_LENGTH(OPTIONS); ++i) {
        defs::index w = 2;
        out << "  " << OPTIONS[i].option;
        w += SIZE(strlen(OPTIONS[i].option));
        if (OPTIONS[i].option_arg != 0) {
            out << " " << OPTIONS[i].option_arg;
            w += 1 + SIZE(strlen(OPTIONS[i].option_arg));
        }

        while (w++ <  max_col + 3)
            out << ' ';
        
        out << OPTIONS[i].description << std::endl;
    }

    out << std::endl;
}

} // namespace ge
