#ifndef GE_ENGINE_OPTIONS
#define GE_ENGINE_OPTIONS

#include "ge/GameWindow.hpp"
#include "ge/Init.hpp"

#include "bl/pair.hpp"
#include "bl/vector.hpp"

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

    bl::vector<bl::pair<CommandType, bl::string>> commands;
    bl::string workingDirectory;
    bool inhibitInitScript;
    bl::string binary;
    bool defaultCD;
    bl::vector<bl::string> shaderDirs;
    bl::vector<bl::string> scriptDirs;
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
