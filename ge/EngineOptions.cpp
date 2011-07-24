#include "opengl.h"
#include "ge/EngineOptions.hpp"

#include "sys/fs/fs.hpp"

#include <cstring>
#include <cstdio>
#include <string>

namespace ge {

namespace {

enum OptionCase {
    Help,
    NoInitScript,
    ScriptDir,
    ShaderDir,
    NoCD,
    Eval,
    Script,
    CWD,
    GLVersion,
    GLProfile,
    GLDebug,
    AASamples,
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
    { "--no-cd", NULL, NoCD, "dont change into the base directory of the program binary" },
    { "--eval", "COMMAND", Eval, "execute COMMAND as a script command" },
    { "--script", "FILE", Script, "load FILE and execute it as a script" },
    { "--cwd", "DIR", CWD, "change into directory DIR" },
    { "--gl-version", "VERSION", GLVersion, "set the OpenGL context version: MAJOR.MINOR" },
    { "--gl-profile", "TYPE", GLProfile, "set the OpenGL profile type: core|compatibility" },
    { "--gl-debug", "BOOL", GLDebug, "create a OpenGL debug context: yes|no" },
    { "--aa-samples", "NUM", AASamples, "set the number of FSAA Samples" },
    { "--glew-experimental", "BOOL", GlewExp, "set the glewExperimental flag: yes|no" }
};

struct State {
    EngineOptions& options;
    const char *program;
    bool no_cd;
    bool no_init;
    
    State(EngineOptions& opts) :
        options(opts),
        program(0),
        no_cd(false),
        no_init(false)
        {}

    bool option(OptionCase opt, const char *arg);
    void end();
};

#define CMDWARN(msg) WARN(std::string("parsing options: ") + (msg))

bool str_eq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

bool State::option(OptionCase opt, const char *arg) {
    switch (OPTIONS[opt].option_case) {
    case Help:
        options.mode = EngineOptions::Help;
        return true;
    case NoInitScript:
        no_init = true;
        return true;
    case ScriptDir:
        options.scriptDirs.push_back(arg);
        return true;
    case ShaderDir:
        options.shaderDirs.push_back(arg);
        return true;
    case NoCD:
        no_cd = true;
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
            options.window.settings.MajorVersion = maj;
            options.window.settings.MinorVersion = min;
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
        if (str_eq(arg, "yes")) {
            options.window.settings.DebugContext = true;
        } else if (str_eq(arg, "no")) {
            options.window.settings.DebugContext = false;
        } else {
            CMDWARN("--gl-debug: not a boolean option");
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
    case GlewExp:
        if (str_eq(arg, "yes")) {
            glewExperimental = GL_TRUE;
        } else if (str_eq(arg, "no")) {
            glewExperimental = GL_FALSE;
        } else {
            CMDWARN("--gl-debug: not a boolean option");
            return false;
        }
        return true;
    default:
        FATAL_ERR("unhandled option_case");
        return false;
    }
}

void State::end() {
    if (!no_cd && options.workingDirectory.empty() && program != 0) {
        std::string bin(program);
        options.workingDirectory = sys::fs::dirname(bin);
        std::string exec = sys::fs::basename(bin);
        if (!exec.empty() && !no_init)
            options.initScript = exec + ".script";
    }
}

} // namespace anon

EngineOptions& EngineOptions::parse(int *argcp, char ***argvp) {

    char **argv = *argvp;
    int argc = *argcp;

    int dest = 1;

    State state(*this);
    if (argc > 0)
        state.program = argv[0];

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

            for (int opt = 0; opt < (int) ARRAY_LENGTH(OPTIONS); ++opt) {
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

    state.end();

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

    for (int i = 0; i < (int) ARRAY_LENGTH(OPTIONS); ++i) {
        int w = 2;
        w += strlen(OPTIONS[i].option);
        if (OPTIONS[i].option_arg != 0)
            w += 1 + strlen(OPTIONS[i].option_arg);
        if (w > max_col)
            max_col = w;
    }

    for (int i = 0; i < (int) ARRAY_LENGTH(OPTIONS); ++i) {
        int w = 2;
        out << "  " << OPTIONS[i].option;
        w += strlen(OPTIONS[i].option);
        if (OPTIONS[i].option_arg != 0) {
            out << " " << OPTIONS[i].option_arg;
            w += 1 + strlen(OPTIONS[i].option_arg);
        }

        while (w++ <  max_col + 3)
            out << ' ';
        
        out << OPTIONS[i].description << std::endl;
    }

    out << std::endl;
}

} // namespace ge
