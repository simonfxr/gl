#ifndef GE_ENGINE_OPTIONS
#define GE_ENGINE_OPTIONS

#include "ge/GameWindow.hpp"
#include "ge/Init.hpp"

#include <bl/vec.hpp>

namespace ge {

struct GE_API EngineOptions
{
    enum CommandType : uint8_t
    {
        Script,
        Command
    };

    enum Mode : uint8_t
    {
        Help,
        Animate
    };

    bl::vec<std::pair<CommandType, bl::string>> commands;
    bl::string workingDirectory;
    bool inhibitInitScript;
    bl::string binary;
    bool defaultCD;
    bl::vec<bl::string> shaderDirs;
    bl::vec<bl::string> scriptDirs;
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
