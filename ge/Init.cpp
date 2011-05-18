#include "opengl.h"

#include "ge/Init.hpp"
#include "ge/Event.hpp"
#include "ge/Engine.hpp"
#include "ge/Commands.hpp"

#include "glt/ShaderManager.hpp"

#include <iostream>

namespace ge {

EngineInitializers::EngineInitializers() {
    initGLEW(*this);
    initInitStats(*this);
    initCommands(*this);
}

static void runInitGLEW(const Event<InitEvent>& e) {
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        e.info.success = false;
    }
    e.info.success = true;
}

void initGLEW(EngineInitializers& inits) {
    inits.init.registerHandler(makeEventHandler(runInitGLEW));
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
    inits.preInit.registerHandler(makeEventHandler(runPreInitStats, initT0));
    inits.postInit.registerHandler(makeEventHandler(runPostInitStats, initT0));
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

void initShaderVersion(EngineInitializers& inits) {
    inits.init.registerHandler(makeEventHandler(runInitShaderVersion));
}

static void runInitCommands(const Event<InitEvent>& e) {
    e.info.success = true;
    using namespace ::ge::commands;
    CommandRegistry& r = e.info.engine.commandRegistry();
    
    r.define("printContextInfo", printContextInfo);
    r.define("reloadShaders", reloadShaders);
    r.define("listBindings", listBindings);
    r.define("bindKey", bindKey);
    r.define("help", help);
    r.define("bindShader", bindShader);
}

void initCommands(EngineInitializers& inits) {
    inits.init.registerHandler(makeEventHandler(runInitCommands));
}

} // namespace ge
