#ifndef GE_ENGINE_OPTIONS
#define GE_ENGINE_OPTIONS

#include "ge/GameWindow.hpp"
#include "ge/Init.hpp"

#include <vector>

namespace ge {

struct GE_API EngineOptions
{

    enum CommandType
    {
        Script,
        Command
    };

    enum Mode
    {
        Help,
        Animate
    };

    std::vector<std::pair<CommandType, std::string>> commands;
    std::string workingDirectory;
    bool inhibitInitScript;
    std::string binary;
    bool defaultCD;
    std::vector<std::string> shaderDirs;
    std::vector<std::string> scriptDirs;
    WindowOptions window;
    Mode mode;
    bool traceOpenGL;
    bool disableRender;
    bool dumpShaders;

    mutable EngineInitializers inits;

    EngineOptions();

    EngineOptions &parse(int *argc, char ***argv);
    static void printHelp();
};

} // namespace ge
#endif
