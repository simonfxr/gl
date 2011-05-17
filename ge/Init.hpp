#ifndef GE_INIT_HPP
#define GE_INIT_HPP

#include "ge/EngineEvents.hpp"
#include "glt/Ref.hpp"

namespace ge {

struct EngineInitializers {
    EventSource<InitEvent> preInit;
    EventSource<InitEvent> init;
    EventSource<InitEvent> postInit;

    EngineInitializers();
};

void initGLEW(EngineInitializers&);

void initInitStats(EngineInitializers&);

} // namespace ge

#endif
