#ifndef GE_ENGINE_HPP
#define GE_ENGINE_HPP

#include "defs.hpp"

#include <string>

#include "ge/EngineOptions.hpp"
#include "ge/GameWindow.hpp"
#include "ge/GameLoop.hpp"
#include "ge/Event.hpp"
#include "ge/EngineEvents.hpp"
#include "ge/Init.hpp"
#include "ge/Command.hpp"
#include "ge/Tokenizer.hpp"
#include "ge/CommandProcessor.hpp"
#include "ge/KeyHandler.hpp"

#include "glt/ShaderManager.hpp"
#include "glt/RenderManager.hpp"

#include "glt/ShaderProgram.hpp"
#include "glt/Uniforms.hpp"

namespace ge {

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

    bool loadStream(Ref<Input>& inp, const std::string& input_name);
    bool loadScript(const std::string& file, bool quiet = false);
    bool evalCommand(const std::string& cmd);

    void addInit(RunLevel lvl, const Ref<EventHandler<InitEvent> >& comm);

    int32 run(const EngineOptions& opts = EngineOptions());

private:
    Engine(const Engine&);
    Engine& operator =(const Engine&);

    struct Data;
    Data * const self;
};

} // namespace ge

#endif
