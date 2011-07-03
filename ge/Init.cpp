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
    std::cerr << "initializing GLEW - experimental option: " << (glewExperimental == GL_TRUE ? "yes" : "no") << std::endl;
    GLenum err = glewInit();

    if (GLEW_OK != err) {
        ERR(std::string("GLEW Error: ") + (const char *) glewGetErrorString(err));
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

    r.define(printContextInfo);
    r.define(reloadShaders);
    r.define(listBindings);
    r.define(bindKey);
    r.define(help);
    r.define(bindShader);
    r.define(initGLDebug);
    r.define(describe);
    r.define(eval);
    r.define(load);
    r.define(addShaderPath);
    r.define(togglePause);
    r.define(perspectiveProjection);
    r.define(postInit);
}

void initCommands(RunLevel lvl, EngineInitializers& inits) {
    inits.reg(lvl, makeEventHandler(runInitCommands));
}

} // namespace ge
