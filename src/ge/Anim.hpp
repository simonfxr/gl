#ifndef GE_ANIM_HPP
#define GE_ANIM_HPP

#include "ge/conf.hpp"

#include "ge/Camera.hpp"
#include "ge/Engine.hpp"
#include "ge/MouseLookPlugin.hpp"

#include <bl/box.hpp>

namespace ge {

template<typename Self, typename... Plugins>
struct Anim : private bl::non_copyable
{
private:
    using self_t = Anim<Self, Plugins...>;
    FixedPluginSet<Plugins...> pluginSet;
    bool _initialized = false;
    bool _linked = false;
    const Self *self() const { return static_cast<const Self *>(this); }
    Self *self() { return static_cast<Self *>(this); }

public:
    bl::box<Engine> engine{ std::in_place, pluginSet };

    Anim() = default;
    virtual ~Anim() = default;

    void link();
    void init(const Event<InitEvent> &);

    virtual void render(const Event<RenderEvent> &) {}
    virtual void animate(const Event<AnimationEvent> &) {}
    virtual void parseArgs(int argc, char *argv[])
    {
        UNUSED(argc);
        UNUSED(argv);
    }

    int main(int argc, char *argv[]);

    template<typename T>
    decltype(auto) get()
    {
        return pluginSet.template get<T>();
    }
};

template<typename Self, typename... Plugins>
using Anim3D =
  Anim<Self, bl::box<Camera>, bl::box<MouseLookPlugin>, Plugins...>;

template<typename Self, typename... Plugins>
int
Anim<Self, Plugins...>::main(int argc, char *argv[])
{
    EngineOptions opts;
    self_t::link();
    self()->link();
    opts.inits.reg(Init, *this, &self_t::init);
    opts.inits.reg(Init, *self(), &Self::init);
    opts.parse(&argc, &argv);
    self()->parseArgs(argc, argv);
    return engine->run(opts);
}

template<typename Self, typename... Plugins>
void
Anim<Self, Plugins...>::link()
{
    ASSERT(pluginSet.allPlugins().size() == sizeof...(Plugins));
    engine->events().render.reg(*self(), &Self::render);
    engine->events().animate.reg(*self(), &Self::animate);
}

template<typename Self, typename... Plugins>
void
Anim<Self, Plugins...>::init(const Event<InitEvent> &ev)
{
    if (!bl::exchange(_linked, true))
        for (auto p : pluginSet.allPlugins())
            engine->enablePlugin(*p);
    ev.info.success = true;
}

} // namespace ge

#endif // GE_ANIM_HPP
