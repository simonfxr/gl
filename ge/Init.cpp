#include "ge/Init.hpp"
#include "ge/Event.hpp"
#include "ge/Engine.hpp"

#include "opengl.h"

#include <iostream>

namespace ge {

EngineInitializers::EngineInitializers() {
    initGLEW(*this);
    initInitStats(*this);
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

static void runPreInitStats(glt::Ref<float> t0, const Event<InitEvent>& e) {
    e.info.success = true;
    *t0 = e.info.engine.now();
}

static void runPostInitStats(glt::Ref<float> t0, const Event<InitEvent>& e) {
    e.info.success = true;
    uint32 ms = uint32((e.info.engine.now() - *t0) * 1000);
    std::cerr << "initialized in " << ms << " ms" << std::endl;
}

void initInitStats(EngineInitializers& inits) {
    glt::Ref<float> initT0(new float);
    inits.preInit.registerHandler(makeEventHandler(runPreInitStats, initT0));
    inits.postInit.registerHandler(makeEventHandler(runPostInitStats, initT0));
}

//         const sf::ContextSettings& c = self->win->GetSettings();

//         std::cerr << "Initialized OpenGL Context"<< std::endl
//                   << "  Version:\t" << c.MajorVersion << "." << c.MinorVersion << std::endl
//                   << "  DepthBits:\t" << c.DepthBits << std::endl
//                   << "  StencilBits:\t" << c.StencilBits << std::endl
//                   << "  Antialiasing:\t" << c.AntialiasingLevel << std::endl
//                   << "  CoreProfile:\t" << (c.CoreProfile ? "yes" : "no") << std::endl
// #ifdef GLDEBUG
//                   << "  DebugContext:\t" << (c.DebugContext ? "yes" : "no") << std::endl
// #endif
//                   << std::endl;


} // namespace ge
