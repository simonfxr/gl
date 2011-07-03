#ifndef GE_ENGINE_OPTIONS
#define GE_ENGINE_OPTIONS

#include "ge/Init.hpp"
#include "ge/GameWindow.hpp"

namespace ge {

struct EngineOptions {

    enum CommandType {
        Script,
        Command
    };

    enum Mode {
        Help,
        Animate
    };
    
    std::vector<std::pair<CommandType, std::string> > commands;
    std::string workingDirectory;
    std::string initScript;
    WindowOptions window;
    Mode mode;

    mutable EngineInitializers inits;

    EngineOptions() : mode(Animate) {}

    EngineOptions& parse(int *argc, char ***argv);
    static void printHelp();
};

} // namespace ge
#endif
