#include "opengl.h"

#include "glt/utils.hpp"

#include "ge/Init.hpp"
#include "ge/Event.hpp"
#include "ge/Engine.hpp"
#include "ge/Commands.hpp"

#include "glt/ShaderManager.hpp"

#include <iostream>

namespace ge {

EngineInitializers::EngineInitializers(bool default_init) {
    if (default_init) {
        initInitStats(*this);
        initCommands(PreInit0, *this);
        initGLEW(PreInit1, *this);
        initShaderVersion(Init, *this);
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
    GLenum err = glewInit();

    if (GLEW_OK != err) {
        ERR(std::string("GLEW Error: ") + (const char *) glewGetErrorString(err));
        e.info.success = false;
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
    std::cerr << "initialized in " << ms << " ms" << std::endl;
}

void initInitStats(EngineInitializers& inits) {
    Ref<float> initT0(new float);
    inits.reg(PreInit0, makeEventHandler(runPreInitStats, initT0));
    inits.reg(PostInit, makeEventHandler(runPostInitStats, initT0));
}

static void runInitShaderVersion(const Event<InitEvent>& e) {
    e.info.success = true;
    const sf::ContextSettings& c = e.info.engine.window().window().GetSettings();
    glt::ShaderManager::ShaderProfile prof = c.CoreProfile ?
        glt::ShaderManager::CoreProfile :
        glt::ShaderManager::CompatibilityProfile;
    
    int vers = c.MajorVersion * 100 + c.MinorVersion * 10;
    e.info.engine.shaderManager().setShaderVersion(vers, prof);
}

void initShaderVersion(RunLevel lvl, EngineInitializers& inits) {
    inits.reg(lvl, makeEventHandler(runInitShaderVersion));
}

static void runInitCommands(const Event<InitEvent>& e) {
    e.info.success = true;
    using namespace ::ge::commands;
    CommandProcessor& r = e.info.engine.commandProcessor();

    r.define("printContextInfo", printContextInfo);
    r.define("reloadShaders", reloadShaders);
    r.define("listBindings", listBindings);
    r.define("bindKey", bindKey);
    r.define("help", help);
    r.define("bindShader", bindShader);
    r.define("load", loadScript);
    r.define("initOpenGLDebug", initGLDebug);
    r.define("describe", describe);
}

void initCommands(RunLevel lvl, EngineInitializers& inits) {
    inits.reg(lvl, makeEventHandler(runInitCommands));
}

} // namespace ge
