#ifndef GE_ENGINE_EVENTS_HPP
#define GE_ENGINE_EVENTS_HPP

#include "ge/Event.hpp"

namespace ge {

struct Engine;

struct GE_API EngineEvent {
    virtual ~EngineEvent();
    Engine& engine;
    EngineEvent(Engine& _engine) : engine(_engine) {}
};

struct GE_API RenderEvent : public EngineEvent {
    float interpolation;
    RenderEvent(Engine& e, float i) : EngineEvent(e), interpolation(i) {}
};

struct GE_API InitEvent : public EngineEvent {
    mutable bool success;
    InitEvent(Engine& e) : EngineEvent(e), success(false) {}
};

struct GE_API ExitEvent : public EngineEvent {
    int32 exitCode;
    ExitEvent(Engine& e, int32 ec) : EngineEvent(e), exitCode(ec) {}
};

struct GE_API  AnimationEvent : public EngineEvent {
    AnimationEvent(Engine& engine) :
        EngineEvent(engine) {}
};

struct GE_API InputEvent : public EngineEvent {
    InputEvent(Engine& engine) :
        EngineEvent(engine) {}
};

struct GE_API  EngineEvents {
    EventSource<InputEvent> handleInput;
    
    EventSource<AnimationEvent> animate;
    
    EventSource<RenderEvent> beforeRender;
    EventSource<RenderEvent> render;
    EventSource<RenderEvent> afterRender;

    EventSource<ExitEvent> exit;
};

} // namespace ge

#endif
