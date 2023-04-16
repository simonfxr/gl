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

#include <memory>

namespace ge {

struct GE_API Engine : private bl::runtime_sized_base
{
    static Engine *construct_at(Engine *, PluginSet &) noexcept;
    static Engine *construct_at(Engine *) noexcept;
    static void destroy_at(Engine *) noexcept;

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

    const std::string &programName() const;

    math::real now();

    void addInit(RunLevel lvl,
                 const std::shared_ptr<EventHandler<InitEvent>> &comm);

    int32_t run(const EngineOptions &opts = EngineOptions());

    void enablePlugin(Plugin &);

    void setDevelDataDir(const std::string &);

    friend size_t invoke(bl::runtime_size_f, bl::type_proxy<Engine>) noexcept;

private:
    struct Data;
    Data &m();
    [[nodiscard]] const Data &m() const;
};

static_assert(bl::runtime_sized<Engine>);

} // namespace ge

#endif
