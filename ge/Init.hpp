#ifndef GE_INIT_HPP
#define GE_INIT_HPP

#include "ge/EngineEvents.hpp"
#include "data/Ref.hpp"

namespace ge {

struct EngineInitializers {
    EventSource<InitEvent> preInit;
    EventSource<InitEvent> init;
    EventSource<InitEvent> postInit;

    EngineInitializers();
};

void initGLEW(EngineInitializers&);

void initInitStats(EngineInitializers&);

void initShaderVersion(EngineInitializers&);

void initCommands(EngineInitializers&);

void initDebug(EngineInitializers&);

} // namespace ge

#endif

