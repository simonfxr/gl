#ifndef GE_EVENT_HPP
#define GE_EVENT_HPP

#include "ge/conf.hpp"
#include "util/functor_traits.hpp"
#include "util/noncopymove.hpp"

#include <memory>
#include <span>
#include <vector>

namespace ge {

template<typename T>
struct Event
{
    using value_type = T;
    T info;
    mutable bool abort;
    bool canAbort;
    Event() : info(), abort(false), canAbort(true) {}
    explicit Event(T &&nfo) : info(std::move(nfo)), abort(false), canAbort(true)
    {}
};

template<typename T>
Event(T &&) -> Event<std::remove_reference_t<std::remove_cv_t<T>>>;

template<typename Ev>
using event_value_type = typename Ev::value_type;

template<typename T>
struct EventHandler
{
    EventHandler() = default;
    EventHandler(const EventHandler &) = default;
    EventHandler(EventHandler &&) noexcept = default;

    EventHandler &operator=(const EventHandler &) = default;
    EventHandler &operator=(EventHandler &&) noexcept = default;

    virtual ~EventHandler() = default;
    virtual void handle(const Event<T> &) = 0;
};

template<typename F, typename T>
struct FunctorEventHandler : public EventHandler<T>
{
private:
    F f;

public:
    FunctorEventHandler(F f_) : f(std::move(f_)) {}
    void handle(const Event<T> &ev) final { f(ev); }
};

template<typename F>
inline auto
makeEventHandler(F f)
{
    using T = event_value_type<std::decay_t<functor_arg_type<F, 0>>>;
    // avoid excessive std::shared_ptr<XX> template instantiations
    return std::shared_ptr<EventHandler<T>>(static_cast<EventHandler<T> *>(
      new FunctorEventHandler<F, T>(std::move(f))));
}

template<typename T, typename E>
inline auto
makeEventHandler(T &o, void (T::*m)(const Event<E> &))
{
    return makeEventHandler([=, &o](const Event<E> &ev) { return (o.*m)(ev); });
}

template<typename T>
struct EventSource : private NonCopyable
{
    inline bool raise(const Event<T> &evnt);
    inline bool reg(std::shared_ptr<EventHandler<T>> handler);

    template<typename... Args>
    bool reg(Args &&...args)
    {
        return reg(makeEventHandler(std::forward<Args>(args)...));
    }

    inline bool unreg(const std::shared_ptr<EventHandler<T>> &handler);
    inline void clear();

    std::span<std::shared_ptr<EventHandler<T>>> handlers()
    {
        return std::span(_handlers);
    }

    std::span<const std::shared_ptr<EventHandler<T>>> handlers() const
    {
        return std::span(_handlers);
    }

private:
    std::vector<std::shared_ptr<EventHandler<T>>> _handlers;
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
EventSource<T>::reg(std::shared_ptr<EventHandler<T>> handler)
{
    if (!handler)
        return true;
    for (auto &h : _handlers)
        if (h == handler)
            return false;
    _handlers.push_back(std::move(handler));
    return true;
}

template<typename T>
bool
EventSource<T>::unreg(const std::shared_ptr<EventHandler<T>> &handler)
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
