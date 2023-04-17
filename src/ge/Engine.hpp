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

#include <bl/new.hpp>

namespace ge {

struct GE_API Engine : bl::runtime_sized_base<Engine>
{
    static Engine &construct_at(void *, PluginSet &) noexcept;
    static Engine &construct_at(void *) noexcept;

    GameWindow &window();
    GameLoop &gameLoop();
    CommandProcessor &commandProcessor();
    KeyHandler &keyHandler();
    ge::ReplServer &replServer();

    glt::ShaderManager &shaderManager();
    glt::RenderManager &renderManager();

    PluginSet &pluginSet();

    EngineEvents &events();

    sys::io::OutStream &out();
    void out(sys::io::OutStream &);

    [[nodiscard]] const std::string &programName() const;

    math::real now();

    void addInit(RunLevel lvl,
                 const std::shared_ptr<EventHandler<InitEvent>> &comm);

    int32_t run(const EngineOptions &opts = EngineOptions());

    void enablePlugin(Plugin &);

    void setDevelDataDir(const std::string &);

    BL_DECLARE_RUNTIME_SIZED(Impl);
};

static_assert(bl::runtime_sized<Engine>);

} // namespace ge

#endif
