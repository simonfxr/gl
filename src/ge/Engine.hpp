#ifndef GE_ENGINE_HPP
#define GE_ENGINE_HPP

#include "defs.hpp"

#include "ge/EngineOptions.hpp"
#include "ge/GameWindow.hpp"
#include "ge/GameLoop.hpp"
#include "ge/Event.hpp"
#include "ge/EngineEvents.hpp"
#include "ge/Init.hpp"
#include "ge/CommandProcessor.hpp"
#include "ge/KeyHandler.hpp"
#include "ge/ReplServer.hpp"

#include "glt/ShaderManager.hpp"
#include "glt/RenderManager.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/Uniforms.hpp"

#include "sys/io/Stream.hpp"

namespace ge {

struct Engine {

    Engine();
    ~Engine();

    GameWindow& window();
    GameLoop& gameLoop();
    CommandProcessor& commandProcessor();
    KeyHandler& keyHandler();
    ge::ReplServer& replServer();
    
    glt::ShaderManager& shaderManager();
    glt::RenderManager& renderManager();

    EngineEvents& events();

    sys::io::OutStream& out();
    void out(sys::io::OutStream&);

    sys::io::OutStream& err();
    void err(sys::io::OutStream&);

    float now();

    void addInit(RunLevel lvl, const Ref<EventHandler<InitEvent> >& comm);

    defs::int32 run(const EngineOptions& opts = EngineOptions());

private:
    Engine(const Engine&);
    Engine& operator =(const Engine&);

    struct Data;
    Data * const self;
};

} // namespace ge

#endif
