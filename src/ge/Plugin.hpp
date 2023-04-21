#ifndef GE_PLUGIN_HPP
#define GE_PLUGIN_HPP

#include "bl/box.hpp"
#include "bl/mp.hpp"
#include "ge/conf.hpp"

#include "err/err.hpp"

#include <bl/string_view.hpp>
#include <span>
#include <unordered_map>

namespace ge {

struct Engine;
struct CommandProcessor;

struct GE_API Plugin
{
    struct TypeInfo
    {
        bl::string_view name;
        size_t size;
    };

    Plugin();
    virtual ~Plugin();

    virtual void registerWith(Engine &);
    virtual void registerCommands(CommandProcessor &);
    virtual const TypeInfo *typeInfo() noexcept = 0;

    template<typename P>
    static const TypeInfo *typeInfoFor() noexcept
    {
        static_assert(std::is_base_of_v<Plugin, P>);
        static_assert(!std::is_abstract_v<P>);

        static const constexpr Plugin::TypeInfo info = {
            { __PRETTY_FUNCTION__,
              []<size_t N>(const char(&)[N]) {
                  return N - 1;
              }(__PRETTY_FUNCTION__) },
            sizeof(P)
        };

        return &info;
    }
};

template<typename P>
struct GE_API PluginImpl : Plugin
{
    PluginImpl() = default;
    ~PluginImpl() override = default;
    const Plugin::TypeInfo *typeInfo() noexcept override
    {
        return Plugin::typeInfoFor<P>();
    }
};

struct GE_API PluginSet
{
    virtual ~PluginSet();
    virtual Plugin *findPluginByType(const Plugin::TypeInfo *) = 0;
    [[nodiscard]] virtual std::span<Plugin *> allPlugins() = 0;

    template<typename P>
    P *findPlugin()
    {
        auto p = findPluginByType(Plugin::typeInfoFor<P>());
        if (!p)
            return nullptr;
        return static_cast<P *>(p);
    }

    template<typename P>
    P *findPluginOrAbort()
    {
        auto p = findPlugin<P>();
        ASSERT_ALWAYS(!!p);
        return p;
    }
};

template<typename... Plugins>
struct GE_API FixedPluginSet final : PluginSet
{
    FixedPluginSet()
    {
        registerPlugins(std::make_index_sequence<sizeof...(Plugins)>{});
        ASSERT(plugin_map.size() == sizeof...(Plugins));
    }

    template<typename P>
    P &get_ref(bl::type_proxy<P>)
    {
        auto &ref = std::get<P>(plugins);
        return ref;
    }

    template<typename P>
    decltype(auto) get()
    {
        if constexpr (decltype(tuple_has_type(
                        bl::type_proxy<std::tuple<Plugins...>>{},
                        bl::type_proxy<P>{}))::value) {
            return std::get<P>(plugins);
        } else {
            return *std::get<bl::box<P>>(plugins);
        }
    }

    Plugin *findPluginByType(const Plugin::TypeInfo *info) override
    {
        if (auto it = plugin_map.find(info); it != plugin_map.end())
            return it->second;
        return nullptr;
    }

    [[nodiscard]] std::span<Plugin *> allPlugins() override
    {
        return { erased_plugins.begin(), erased_plugins.size() };
    }

private:
    std::tuple<Plugins...> plugins;
    std::array<Plugin *, sizeof...(Plugins)> erased_plugins{};
    std::unordered_map<const Plugin::TypeInfo *, Plugin *> plugin_map{};

    template<size_t... Is>
    void registerPlugins(std::index_sequence<Is...>)
    {
        (..., registerPlugin<Is>());
    }

    template<typename T>
    decltype(auto) unwrap_plugin(bl::box<T> &p)
    {
        return *p;
    }

    decltype(auto) unwrap_plugin(auto &p)
    {
        (void) static_cast<Plugin &>(p);
        return p;
    }

    template<size_t I>
    void registerPlugin()
    {
        auto &plugin = std::get<I>(plugins);
        auto &up = unwrap_plugin(plugin);
        erased_plugins[I] = &up;
        plugin_map[up.typeInfo()] = &up;
    }

    template<typename U, typename... Ts>
    constexpr auto tuple_has_type(bl::type_proxy<std::tuple<Ts...>>,
                                  bl::type_proxy<U>)
    {
        if constexpr ((std::is_same_v<U, Ts> || ...)) {
            return std::true_type{};
        } else {
            return std::false_type{};
        }
    }
};

} // namespace ge

#endif
