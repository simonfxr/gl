#ifndef GE_EVENT_HPP
#define GE_EVENT_HPP

#include "bl/array_view.hpp"
#include "bl/shared_ptr.hpp"
#include "bl/vector.hpp"
#include "ge/conf.hpp"
#include "util/functor_traits.hpp"

namespace ge {

template<typename T>
struct Event
{
    using value_type = T;
    T info;
    mutable bool abort;
    bool canAbort;
    Event() : info(), abort(false), canAbort(true) {}
    explicit Event(T &&nfo) : info(bl::move(nfo)), abort(false), canAbort(true)
    {}
};

template<typename T>
Event(T)->Event<T>;

template<typename Ev>
using event_value_type = typename Ev::value_type;

template<typename T>
struct EventHandler
{
    EventHandler() = default;
    EventHandler(const EventHandler &) = default;
    EventHandler(EventHandler &&) = default;

    EventHandler &operator=(const EventHandler &) = default;
    EventHandler &operator=(EventHandler &&) = default;

    virtual ~EventHandler() = default;
    virtual void handle(const Event<T> &ev) = 0;
};

template<typename F, typename T>
struct FunctorEventHandler : public EventHandler<T>
{
private:
    F f;

public:
    FunctorEventHandler(F f_) : f(bl::move(f_)) {}
    void handle(const Event<T> &ev) final override { f(ev); }
};

template<typename F>
inline auto
makeEventHandler(F f)
{
    using T = event_value_type<bl::decay_t<functor_arg_type<F, 0>>>;
    // avoid excessive bl::shared_ptr<XX> template instantiations
    return bl::shared_ptr<EventHandler<T>>(
      bl::shared_from_ptr_t{},
      static_cast<EventHandler<T> *>(
        new FunctorEventHandler<F, T>(bl::move(f))));
}

template<typename T, typename E>
inline auto
makeEventHandler(T &o, void (T::*m)(const Event<E> &))
{
    return makeEventHandler([=, &o](const Event<E> &ev) { return (o.*m)(ev); });
}

template<typename T>
struct EventSource
{
    inline bool raise(const Event<T> &evnt);
    inline bool reg(bl::shared_ptr<EventHandler<T>> handler);

    template<typename... Args>
    bool reg(Args &&... args)
    {
        return reg(makeEventHandler(bl::forward<Args>(args)...));
    }

    inline bool unreg(const bl::shared_ptr<EventHandler<T>> &handler);
    inline void clear();

    EventSource() = default;
    EventSource(const EventSource<T> &) = delete;
    EventSource<T> &operator=(const EventSource<T> &) = delete;

    bl::array_view<bl::shared_ptr<EventHandler<T>>> handlers()
    {
        return _handlers;
    }

    bl::array_view<const bl::shared_ptr<EventHandler<T>>> handlers() const
    {
        return view_array(_handlers);
    }

private:
    bl::vector<bl::shared_ptr<EventHandler<T>>> _handlers;
};

template<typename T>
bool
EventSource<T>::raise(const Event<T> &e)
{
    for (auto &h : _handlers) {
        h->handle(e);
        if (e.canAbort && e.abort)
            return false;
    }

    return true;
}

template<typename T>
bool
EventSource<T>::reg(bl::shared_ptr<EventHandler<T>> handler)
{
    if (!handler)
        return true;
    for (auto &h : _handlers)
        if (h == handler)
            return false;
    _handlers.push_back(bl::move(handler));
    return true;
}

template<typename T>
bool
EventSource<T>::unreg(const bl::shared_ptr<EventHandler<T>> &handler)
{
    if (!handler)
        return true;
    for (size_t i = 0; i < _handlers.size(); ++i) {
        if (_handlers[i] == handler) {
            _handlers.erase(_handlers.begin() + i);
            return true;
        }
    }
    return false;
}

template<typename T>
void
EventSource<T>::clear()
{
    _handlers.clear();
}

} // namespace ge

#endif
