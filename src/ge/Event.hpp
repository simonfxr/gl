#ifndef GE_EVENT_HPP
#define GE_EVENT_HPP

#include "ge/conf.hpp"

#include <memory>
#include <vector>

namespace ge {

using namespace defs;

template<typename T>
struct Event
{
    T info;
    mutable bool abort;
    bool canAbort;
    Event() : info(), abort(false), canAbort(true) {}
    explicit Event(const T &_info) : info(_info), abort(false), canAbort(true)
    {}
};

template<typename T>
Event<T>
makeEvent(const T &info)
{
    return Event<T>(info);
}

template<typename T>
struct EventHandler
{
    virtual ~EventHandler() {}
    virtual void handle(const Event<T> &ev) = 0;
};

template<typename F, typename E>
struct FunctorHandler : public EventHandler<E>
{
private:
    F f;

public:
    explicit FunctorHandler(const F &_f) : f(_f) {}
    virtual void handle(const Event<E> &ev) final override { f(ev); }
};

template<typename F, typename E>
std::shared_ptr<EventHandler<E>>
makeEventHandler(F f)
{
    return makeRef(new FunctorHandler<F, E>(f));
}

template<typename E>
std::shared_ptr<EventHandler<E>>
makeEventHandler(void (*fun)(const Event<E> &))
{
    return std::shared_ptr<EventHandler<E>>(
      new FunctorHandler<void (*)(const Event<E> &), E>(fun));
}

template<typename S, typename F, typename E>
struct FunctorStateHandler : public EventHandler<E>
{
private:
    S s;
    F f;

public:
    FunctorStateHandler(S _s, const F &_f) : s(_s), f(_f) {}
    virtual void handle(const Event<E> &ev) final override { f(s, ev); }
};

template<typename S, typename E>
std::shared_ptr<EventHandler<E>>
makeEventHandler(void (*fun)(S s, const Event<E> &), S s)
{
    return std::shared_ptr<EventHandler<E>>(
      new FunctorStateHandler<S, void (*)(S s, const Event<E> &), E>(s, fun));
}

template<typename T, typename M, typename E>
struct MemberFunHandler : public EventHandler<E>
{
private:
    T *o;
    M m;

public:
    MemberFunHandler(T *_o, M _m) : o(_o), m(_m) {}
    virtual void handle(const Event<E> &ev) final override { (o->*m)(ev); }
};

template<typename T, typename E>
std::shared_ptr<EventHandler<E>>
makeEventHandler(T *o, void (T::*m)(const Event<E> &))
{
    return std::shared_ptr<EventHandler<E>>(
      new MemberFunHandler<T, void (T::*)(const Event<E> &), E>(o, m));
}

template<typename F, typename E>
struct VoidFunctorHandler : public EventHandler<E>
{
private:
    F f;

public:
    explicit VoidFunctorHandler(const F &_f) : f(_f) {}
    virtual void handle(const Event<E> &ev) final override
    {
        UNUSED(ev);
        f();
    }
};

template<typename F, typename E>
std::shared_ptr<EventHandler<E>>
makeVoidEventHandler(F f)
{
    return makeRef(new VoidFunctorHandler<F, E>(f));
}

template<typename T>
struct EventSource
{
    bool raise(const Event<T> &evnt);
    bool reg(const std::shared_ptr<EventHandler<T>> &handler);
    bool unreg(const std::shared_ptr<EventHandler<T>> &handler);
    void clear();

    std::vector<std::shared_ptr<EventHandler<T>>> handlers;
    EventSource() {}

private:
    EventSource(const EventSource<T> &);
    EventSource<T> &operator=(const EventSource<T> &);
};

template<typename T>
bool
EventSource<T>::raise(const Event<T> &e)
{
    for (defs::index i = 0; i < SIZE(handlers.size()); ++i) {
        handlers[size_t(i)]->handle(e);
        if (e.canAbort && e.abort)
            return false;
    }

    return true;
}

template<typename T>
bool
EventSource<T>::reg(const std::shared_ptr<EventHandler<T>> &handler)
{
    if (!handler)
        return true;
    for (defs::index i = 0; i < SIZE(handlers.size()); ++i)
        if (handlers[size_t(i)] == handler)
            return false;
    handlers.push_back(handler);
    return true;
}

template<typename T>
bool
EventSource<T>::unreg(const std::shared_ptr<EventHandler<T>> &handler)
{
    if (!handler)
        return true;
    for (defs::index i = 0; i < SIZE(handlers.size()); ++i)
        if (handlers[size_t(i)] == handler) {
            handlers.erase(handlers.begin() + i);
            return true;
        }
    return false;
}

template<typename T>
void
EventSource<T>::clear()
{
    handlers.clear();
}

} // namespace ge

#endif
