#include "ge/Init.hpp"

#include "ge/Commands.hpp"
#include "ge/Engine.hpp"
#include "ge/Event.hpp"
#include "glt/ShaderManager.hpp"
#include "glt/utils.hpp"

namespace ge {

EngineInitializers::EngineInitializers(bool default_init)
{
    if (default_init) {
        initInitStats(*this);
        initCommands(PreInit0, *this);
        initMemInfo(PreInit1, *this);
        initShaderVersion(PreInit1, *this);
    }
}

void
EngineInitializers::reg(RunLevel lvl,
                        std::shared_ptr<EventHandler<InitEvent>> handler)
{
    switch (lvl) {
    case PreInit0:
        preInit0.reg(std::move(handler));
        break;
    case PreInit1:
        preInit1.reg(std::move(handler));
        break;
    case Init:
        init.reg(std::move(handler));
        break;
    case PostInit:
        postInit.reg(std::move(handler));
        break;
    }
}

void
initInitStats(EngineInitializers &inits)
{
    auto initT0 = std::make_shared<math::real>();
    inits.reg(PreInit0, [=](const Event<InitEvent> &e) {
        e.info.success = true;
        *initT0 = e.info.engine.now();
    });

    inits.reg(PostInit, [=](const Event<InitEvent> &e) {
        e.info.success = true;
        auto ms = uint32_t((e.info.engine.now() - *initT0) * 1000);
        e.info.engine.out() << "initialized in " << ms << " ms\n";
    });
}

void
initMemInfo(RunLevel lvl, EngineInitializers &inits)
{
    inits.reg(lvl, [](const Event<InitEvent> &e) {
        e.info.success = true;
        glt::GLMemInfoATI::init();
        glt::GLMemInfoNV::init();
    });
}

void
initShaderVersion(RunLevel lvl, EngineInitializers &inits)
{
    inits.reg(lvl, [](const Event<InitEvent> &e) {
        e.info.success = true;
        auto c = e.info.engine.window().contextInfo();
        auto prof = c.coreProfile ? glt::ShaderProfile::Core
                                  : glt::ShaderProfile::Compatibility;

        auto vers =
          glt::ShaderManager::glToShaderVersion(c.majorVersion, c.minorVersion);
        e.info.engine.shaderManager().setShaderVersion(vers, prof);
    });
}

void
initCommands(RunLevel lvl, EngineInitializers &inits)
{
    inits.reg(lvl, [](const Event<InitEvent> &ev) {
        ev.info.success = true;
        CommandProcessor &r = ev.info.engine.commandProcessor();
        registerCommands(r);
    });
}

} // namespace ge
