#include "opengl.hpp"

#include "glt/utils.hpp"

#include "ge/Init.hpp"
#include "ge/Event.hpp"
#include "ge/Engine.hpp"
#include "ge/Commands.hpp"

#include "glt/ShaderManager.hpp"

namespace ge {

EngineInitializers::EngineInitializers(bool default_init) {
    if (default_init) {
        initInitStats(*this);
        initCommands(PreInit0, *this);
        initGLEW(PreInit1, *this);
        initMemInfo(PreInit1, *this);
        initShaderVersion(PreInit1, *this);
    }
}

void EngineInitializers::reg(RunLevel lvl, const Ref<EventHandler<InitEvent> >& handler) {
    switch (lvl) {
    case PreInit0: preInit0.reg(handler); break;
    case PreInit1: preInit1.reg(handler); break;
    case Init: init.reg(handler); break;
    case PostInit: postInit.reg(handler); break;
    default:
        ERR("invalid runlevel");
    }
}

static void runInitGLEW(const Event<InitEvent>& e) {
    e.info.engine.out() << "initializing GLEW - experimental option: " << (glt::gl_unbool(glewExperimental) ? "yes" : "no") << sys::io::endl;
    GLenum err = glewInit();

    if (GLEW_OK != err) {
        ERR(std::string("GLEW Error: ") + glt::gl_unstr(glewGetErrorString(err)));
        return;
    }

    e.info.success = true;
}

void initGLEW(RunLevel lvl, EngineInitializers& inits) {
    inits.reg(lvl, makeEventHandler(runInitGLEW));
}

static void runPreInitStats(Ref<float> t0, const Event<InitEvent>& e) {
    e.info.success = true;
    *t0 = e.info.engine.now();
}

static void runPostInitStats(Ref<float> t0, const Event<InitEvent>& e) {
    e.info.success = true;
    uint32 ms = uint32((e.info.engine.now() - *t0) * 1000);
    e.info.engine.out() << "initialized in " << ms << " ms" << sys::io::endl;
}

void initInitStats(EngineInitializers& inits) {
    Ref<float> initT0(new float);
    inits.reg(PreInit0, makeEventHandler(runPreInitStats, initT0));
    inits.reg(PostInit, makeEventHandler(runPostInitStats, initT0));
}

static void runInitMemInfo(const Event<InitEvent>& e) {
    e.info.success = true;
    glt::GLMemInfoATI::init();
    glt::GLMemInfoNV::init();
}

void initMemInfo(RunLevel lvl, EngineInitializers& inits) {
    inits.reg(lvl, makeEventHandler(runInitMemInfo));
}

static void runInitShaderVersion(const Event<InitEvent>& e) {
    e.info.success = true;
    ge::GLContextInfo c;
    e.info.engine.window().contextInfo(c);
    glt::ShaderManager::ShaderProfile prof = c.coreProfile ?
        glt::ShaderManager::CoreProfile :
        glt::ShaderManager::CompatibilityProfile;
    
    unsigned vers = c.majorVersion * 100 + c.minorVersion * 10;
    e.info.engine.shaderManager().setShaderVersion(vers, prof);
}

void initShaderVersion(RunLevel lvl, EngineInitializers& inits) {
    inits.reg(lvl, makeEventHandler(runInitShaderVersion));
}

static void runInitCommands(const Event<InitEvent>& e) {
    e.info.success = true;
    CommandProcessor& r = e.info.engine.commandProcessor();
    const Commands& cs = commands();

    r.define(cs.printContextInfo);
    r.define(cs.printMemInfo);
    r.define(cs.reloadShaders);
    r.define(cs.listCachedShaders);
    r.define(cs.listBindings);
    r.define(cs.bindKey);
    r.define(cs.help);
    r.define(cs.bindShader);
    r.define(cs.initGLDebug);
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

void initCommands(RunLevel lvl, EngineInitializers& inits) {
    inits.reg(lvl, makeEventHandler(runInitCommands));
}

} // namespace ge
