#ifndef GE_INIT_HPP
#define GE_INIT_HPP

#include "ge/EngineEvents.hpp"

#include <memory>

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

    void reg(RunLevel lvl, std::shared_ptr<EventHandler<InitEvent>> handler);

    template<typename... Args>
    void reg(RunLevel lvl, Args &&... args)
    {
        reg(lvl, makeEventHandler(std::forward<Args>(args)...));
    }
};

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
