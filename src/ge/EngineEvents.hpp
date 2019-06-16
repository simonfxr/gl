#ifndef GE_ENGINE_EVENTS_HPP
#define GE_ENGINE_EVENTS_HPP

#include "ge/Event.hpp"
#include "math/real.hpp"

namespace ge {

struct Engine;

struct GE_API EngineEvent
{
    Engine &engine;
    explicit constexpr EngineEvent(Engine &e) : engine(e) {}
};

struct GE_API RenderEvent : public EngineEvent
{
    math::real interpolation;
    RenderEvent(Engine &e, math::real i) : EngineEvent(e), interpolation(i) {}
};

struct GE_API InitEvent : public EngineEvent
{
    mutable bool success;
    explicit InitEvent(Engine &e) : EngineEvent(e), success(false) {}
};

struct GE_API ExitEvent : public EngineEvent
{
    int32_t exitCode;
    ExitEvent(Engine &e, int32_t ec) : EngineEvent(e), exitCode(ec) {}
};

struct GE_API AnimationEvent : public EngineEvent
{
    explicit AnimationEvent(Engine &e) : EngineEvent(e) {}
};

struct GE_API InputEvent : public EngineEvent
{
    explicit InputEvent(Engine &e) : EngineEvent(e) {}
};

struct GE_API EngineEvents
{
    EventSource<InputEvent> handleInput;

    EventSource<AnimationEvent> animate;

    EventSource<RenderEvent> beforeRender;
    EventSource<RenderEvent> render;
    EventSource<RenderEvent> afterRender;

    EventSource<ExitEvent> exit;
};

} // namespace ge

#endif
