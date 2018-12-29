#ifndef GE_INIT_HPP
#define GE_INIT_HPP

#include "data/Ref.hpp"
#include "ge/EngineEvents.hpp"

namespace ge {

enum RunLevel
{
    PreInit0,
    PreInit1,
    Init,
    PostInit
};

struct GE_API EngineInitializers
{

    EventSource<InitEvent> preInit0;
    EventSource<InitEvent> preInit1;
    EventSource<InitEvent> init;
    EventSource<InitEvent> postInit;

    EngineInitializers(bool default_init = true);

    void reg(RunLevel lvl, const Ref<EventHandler<InitEvent>> &handler);
};

GE_API void
initGLEW(RunLevel lvl, EngineInitializers &);

GE_API void
initShaderVersion(RunLevel lvl, EngineInitializers &);

GE_API void
initCommands(RunLevel lvl, EngineInitializers &);

GE_API void
initMemInfo(RunLevel lvl, EngineInitializers &);

GE_API void
initInitStats(EngineInitializers &inits);

} // namespace ge

#endif
