#ifndef GE_ENGINE_HPP
#define GE_ENGINE_HPP

#include "ge/conf.hpp"

#include "ge/CommandProcessor.hpp"
#include "ge/EngineEvents.hpp"
#include "ge/EngineOptions.hpp"
#include "ge/Event.hpp"
#include "ge/GameLoop.hpp"
#include "ge/GameWindow.hpp"
#include "ge/Init.hpp"
#include "ge/KeyHandler.hpp"
#include "ge/Plugin.hpp"
#include "ge/ReplServer.hpp"
#include "glt/RenderManager.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/ShaderProgram.hpp"
#include "glt/Uniforms.hpp"
#include "sys/io/Stream.hpp"

namespace ge {

struct GE_API Engine
{
    Engine();
    ~Engine();

    GameWindow &window();
    GameLoop &gameLoop();
    CommandProcessor &commandProcessor();
    KeyHandler &keyHandler();
    ge::ReplServer &replServer();

    glt::ShaderManager &shaderManager();
    glt::RenderManager &renderManager();

    EngineEvents &events();

    sys::io::OutStream &out();
    void out(sys::io::OutStream &);

    const bl::string programName() const;

    math::real now();

    void addInit(RunLevel lvl,
                 const bl::shared_ptr<EventHandler<InitEvent>> &comm);

    int32_t run(const EngineOptions &opts = EngineOptions());

    void enablePlugin(Plugin &);

    void setDevelDataDir(const bl::string &);

private:
    DECLARE_PIMPL(GE_API, self);
};

} // namespace ge

#endif
