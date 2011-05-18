#ifndef GE_EVENT_HPP
#define GE_EVENT_HPP

#include "defs.h"
#include "data/Ref.hpp"

#include <vector>

namespace ge {

struct Unit {};

template <typename T>
struct Event {
    T info;
    Event() {}
    Event(const T& _info) : info(_info) {}
};

template <typename T>
Event<T> makeEvent(const T& info) {
    return Event<T>(info);
}

template <typename T>
struct EventHandler {
    virtual ~EventHandler() {}
    virtual void handle(const Event<T>& ev) = 0;
};

template <typename F, typename E>
struct FunctorHandler EXPLICIT : public EventHandler<E> {
private:
    F f;
public:
    FunctorHandler(const F& _f) : f(_f) {}
    void handle(const Event<E>& ev) OVERRIDE { f(ev); }
};

template <typename F, typename E>
Ref<EventHandler<E> > makeEventHandler(F f) {
    return makeRef(new FunctorHandler<F, E>(f));
}

template <typename E>
Ref<EventHandler<E> > makeEventHandler(void (*fun)(const Event<E>&)) {
    return Ref<EventHandler<E> >(new FunctorHandler<void (*)(const Event<E>&), E>(fun));
}

template <typename S, typename F, typename E>
struct FunctorStateHandler : public EventHandler<E> {
private:
    S s;
    F f;    
public:
    FunctorStateHandler(S _s, const F& _f) : s(_s), f(_f) {}
    void handle(const Event<E>& ev) OVERRIDE { f(s, ev); }
};

template <typename S, typename E>
Ref<EventHandler<E> > makeEventHandler(void (*fun)(S s, const Event<E>&), S s) {
    return Ref<EventHandler<E> >(new FunctorStateHandler<S, void (*)(S s, const Event<E>&), E>(s, fun));
}


template <typename F, typename E>
struct VoidFunctorHandler EXPLICIT : public EventHandler<E> {
private:
    F f;
public:
    VoidFunctorHandler(const F& _f) : f(_f) {}
    void handle(const Event<E>& ev) OVERRIDE { UNUSED(ev); f(); }
};

template <typename F, typename E>
Ref<EventHandler<E> > makeVoidEventHandler(F f) {
    return makeRef(new VoidFunctorHandler<F, E>(f));
}


template <typename T>
struct EventSource {
    void raise(const Event<T>& evnt);
    bool registerHandler(const Ref<EventHandler<T> >& handler);
    bool unregisterHandler(const Ref<EventHandler<T> >& handler);
    void clearHandlers();
    
    std::vector<Ref<EventHandler<T> > > handlers;
    EventSource() {}
private:
    EventSource(const EventSource<T>&);
    EventSource<T>& operator =(const EventSource<T>&);
};

template <typename T>
void EventSource<T>::raise(const Event<T>& e) {
    for (uint32 i = 0; i < handlers.size(); ++i)
        handlers[i]->handle(e);
}

template <typename T>
bool EventSource<T>::registerHandler(const Ref<EventHandler<T> >& handler) {
    if (handler.ptr() == 0)
        return true;
    for (uint32 i = 0; i < handlers.size(); ++i)
        if (handlers[i].ptr() == handler.ptr())
            return false;
    handlers.push_back(handler);
    return true;
}

template <typename T>
bool EventSource<T>::unregisterHandler(const Ref<EventHandler<T> >& handler) {
    if (handler.ptr() == 0)
        return true;
    for (uint32 i = 0; i < handlers.size(); ++i)
        if (handlers[i].ptr() == handler.ptr()) {
            handlers.erase(handlers.begin() + i);
            return true;
        }
    return false;
}

template <typename T>
void EventSource<T>::clearHandlers() {
    handlers.clear();
}

} // namespace ge

#endif
