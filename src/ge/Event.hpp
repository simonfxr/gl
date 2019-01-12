#ifndef GE_EVENT_HPP
#define GE_EVENT_HPP

#include "ge/conf.hpp"

#include <memory>
#include <type_traits>
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

template<typename Ev>
using value_type_of = typename Ev::value_type;

template<typename T>
struct EventHandler
{
    virtual ~EventHandler() {}
    virtual void handle(const Event<T> &ev) = 0;
};

template<typename T>
struct functor_traits_impl
{};

template<typename Functor, typename Ret, typename... Args>
struct functor_traits_impl<Ret (Functor::*)(Args...) const>
{
    static inline constexpr size_t arity = sizeof...(Args);
    using result_type = Ret;

    template<size_t N>
    struct arg_type
    {
        using type = std::tuple_element_t<N, std::tuple<Args...>>;
    };
};

template<typename F>
struct functor_traits : functor_traits_impl<decltype(&F::operator())>
{};

template<typename F, size_t i>
using functor_arg_type = typename functor_traits<F>::template arg_type<i>::type;

template<typename F, typename T>
struct FunctorEventHandler : public EventHandler<T>
{
private:
    F f;

public:
    FunctorEventHandler(F f_) : f(std::move(f_)) {}
    virtual void handle(const Event<T> &ev) final override { f(ev); }
};

template<typename F>
auto
makeEventHandler(F f)
{
    using T = value_type_of<std::decay_t<functor_arg_type<F, 0>>>;
    return static_cast<std::shared_ptr<EventHandler<T>>>(
      std::make_shared<FunctorEventHandler<F, T>>(std::move(f)));
}

template<typename T>
auto
makeEventHandler(void (*handler)(const Event<T> &))
{
    return makeEventHandler([h = handler](const Event<T> &ev) { h(ev); });
}

template<typename T, typename E>
auto
makeEventHandler(T &o, void (T::*m)(const Event<E> &))
{
    return makeEventHandler([=, &o](const Event<E> &ev) { return (o.*m)(ev); });
}

template<typename T>
struct EventSource
{
    bool raise(const Event<T> &evnt);
    bool reg(std::shared_ptr<EventHandler<T>> handler);

    template<typename... Args>
    bool reg(Args &&... args)
    {
        return reg(makeEventHandler(std::forward<Args>(args)...));
    }

    bool unreg(const std::shared_ptr<EventHandler<T>> &handler);
    void clear();

    std::vector<std::shared_ptr<EventHandler<T>>> handlers;

    EventSource() = default;

    EventSource(const EventSource<T> &) = delete;
    EventSource<T> &operator=(const EventSource<T> &) = delete;
};

template<typename T>
bool
EventSource<T>::raise(const Event<T> &e)
{
    for (auto &h : handlers) {
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
    for (auto &h : handlers)
        if (h == handler)
            return false;
    handlers.push_back(std::move(handler));
    return true;
}

template<typename T>
bool
EventSource<T>::unreg(const std::shared_ptr<EventHandler<T>> &handler)
{
    if (!handler)
        return true;
    for (size_t i = 0; i < handlers.size(); ++i) {
        if (handlers[i] == handler) {
            handlers.erase(handlers.begin() + i);
            return true;
        }
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
