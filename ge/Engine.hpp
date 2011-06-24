#ifndef GE_ENGINE_HPP
#define GE_ENGINE_HPP

#include "defs.h"

#include <string>
#include <istream>

#include "ge/GameWindow.hpp"
#include "ge/GameLoop.hpp"
#include "ge/Event.hpp"
#include "ge/EngineEvents.hpp"
#include "ge/Init.hpp"
#include "ge/CommandProcessor.hpp"
#include "ge/KeyHandler.hpp"

#include "glt/ShaderManager.hpp"
#include "glt/RenderManager.hpp"

#include "glt/ShaderProgram.hpp"
#include "glt/Uniforms.hpp"

namespace ge {

struct EngineOpts {

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
    WindowOpts window;
    Mode mode;

    mutable EngineInitializers inits;

    EngineOpts() : mode(Animate) {}

    EngineOpts& parseOpts(int *argc, char ***argv);
};

struct Engine {

    Engine();
    ~Engine();

    GameWindow& window();
    GameLoop& gameLoop();
    CommandProcessor& commandProcessor();
    KeyHandler& keyHandler();
    
    glt::ShaderManager& shaderManager();
    glt::RenderManager& renderManager();
    
    EngineEvents& events();

    float now();

    bool loadStream(std::istream& inp, const std::string& input_name);
    bool loadScript(const std::string& file, bool quiet = false);
    bool evalCommand(const std::string& cmd);

    int32 run(const EngineOpts& opts = EngineOpts());

private:
    Engine(const Engine&);
    Engine& operator =(const Engine&);

    struct Data;
    Data * self;
};

} // namespace ge

#endif
