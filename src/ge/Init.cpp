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
                        const std::shared_ptr<EventHandler<InitEvent>> &handler)
{
    switch (lvl) {
    case PreInit0:
        preInit0.reg(handler);
        break;
    case PreInit1:
        preInit1.reg(handler);
        break;
    case Init:
        init.reg(handler);
        break;
    case PostInit:
        postInit.reg(handler);
        break;
    }
}

static void
runPreInitStats(std::shared_ptr<math::real> t0, const Event<InitEvent> &e)
{
    e.info.success = true;
    *t0 = e.info.engine.now();
}

static void
runPostInitStats(std::shared_ptr<math::real> t0, const Event<InitEvent> &e)
{
    e.info.success = true;
    auto ms = uint32_t((e.info.engine.now() - *t0) * 1000);
    e.info.engine.out() << "initialized in " << ms << " ms" << sys::io::endl;
}

void
initInitStats(EngineInitializers &inits)
{
    auto initT0 = std::make_shared<math::real>();
    inits.reg(PreInit0, makeEventHandler(runPreInitStats, initT0));
    inits.reg(PostInit, makeEventHandler(runPostInitStats, initT0));
}

static void
runInitMemInfo(const Event<InitEvent> &e)
{
    e.info.success = true;
    glt::GLMemInfoATI::init();
    glt::GLMemInfoNV::init();
}

void
initMemInfo(RunLevel lvl, EngineInitializers &inits)
{
    inits.reg(lvl, makeEventHandler(runInitMemInfo));
}

static void
runInitShaderVersion(const Event<InitEvent> &e)
{
    e.info.success = true;
    auto c = e.info.engine.window().contextInfo();
    auto prof = c.coreProfile ? glt::ShaderProfile::Core
                              : glt::ShaderProfile::Compatibility;

    auto vers =
      glt::ShaderManager::glToShaderVersion(c.majorVersion, c.minorVersion);
    e.info.engine.shaderManager().setShaderVersion(vers, prof);
}

void
initShaderVersion(RunLevel lvl, EngineInitializers &inits)
{
    inits.reg(lvl, makeEventHandler(runInitShaderVersion));
}

static void
runInitCommands(const Event<InitEvent> &e)
{
    e.info.success = true;
    CommandProcessor &r = e.info.engine.commandProcessor();
    const Commands &cs = commands();

    r.define(cs.printContextInfo);
    r.define(cs.printMemInfo);
    r.define(cs.reloadShaders);
    r.define(cs.listCachedShaders);
    r.define(cs.listBindings);
    r.define(cs.bindKey);
    r.define(cs.help);
    r.define(cs.bindShader);
    r.define(cs.initGLDebug);
    r.define(cs.ignoreGLDebugMessage);
    r.define(cs.describe);
    r.define(cs.eval);
    r.define(cs.load);
    r.define(cs.addShaderPath);
    r.define(cs.prependShaderPath);
    r.define(cs.removeShaderPath);
    r.define(cs.togglePause);
    r.define(cs.perspectiveProjection);
    r.define(cs.postInit);
    r.define(cs.startReplServer);
    r.define(cs.printGLInstanceStats);
}

void
initCommands(RunLevel lvl, EngineInitializers &inits)
{
    inits.reg(lvl, makeEventHandler(runInitCommands));
}

} // namespace ge
