#ifndef GE_ENGINE_EVENTS_HPP
#define GE_ENGINE_EVENTS_HPP

#include "ge/Event.hpp"

namespace ge {

struct Engine;

struct EngineEvent {
    Engine& engine;
    EngineEvent(Engine& _engine) : engine(_engine) {}
};

struct RenderEvent : public EngineEvent {
    float interpolation;
    RenderEvent(Engine& e, float i) : EngineEvent(e), interpolation(i) {}
};

struct InitEvent : public EngineEvent {
    mutable bool success;
    InitEvent(Engine& e) : EngineEvent(e), success(false) {}
};

struct ExitEvent : public EngineEvent {
    int32 exitCode;
    ExitEvent(Engine& e, int32 ec) : EngineEvent(e), exitCode(ec) {}
};

struct EngineEvents {
    EventSource<EngineEvent> handleInput;
    
    EventSource<EngineEvent> animate;
    
    EventSource<RenderEvent> beforeRender;
    EventSource<RenderEvent> render;
    EventSource<RenderEvent> afterRender;

    EventSource<ExitEvent> exit;
};

} // namespace ge

#endif
