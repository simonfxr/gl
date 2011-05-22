#include "opengl.h"

#include "glt/utils.hpp"

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
    initShaderVersion(*this);
    initDebug(*this);
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
    inits.init.reg(makeEventHandler(runInitGLEW));
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
    inits.preInit.reg(makeEventHandler(runPreInitStats, initT0));
    inits.postInit.reg(makeEventHandler(runPostInitStats, initT0));
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
    inits.init.reg(makeEventHandler(runInitShaderVersion));
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
}

void initCommands(EngineInitializers& inits) {
    inits.init.reg(makeEventHandler(runInitCommands));
}

static void runInitDebug(const Event<InitEvent>& e) {
    e.info.success = true;
    if (e.info.engine.window().window().GetSettings().DebugContext) {
        std::cerr << "initalizing OpenGL debug output" << std::endl;
        glt::initDebug();
    } else {
        std::cerr << "cannot initialize OpenGL debug output: no debug context" << std::endl;
    }
}

void initDebug(EngineInitializers& inits) {
    inits.init.reg(makeEventHandler(runInitDebug));
}

} // namespace ge
