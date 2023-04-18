#ifndef GE_MOUSE_LOOK_PLUGIN_HPP
#define GE_MOUSE_LOOK_PLUGIN_HPP

#include "ge/Camera.hpp"
#include "ge/Plugin.hpp"

#include <bl/new.hpp>

namespace ge {

struct GE_API MouseLookPlugin
  : private bl::runtime_sized_base<MouseLookPlugin>
  , public PluginImpl<MouseLookPlugin>
{
    static MouseLookPlugin &construct_at(void *) noexcept;

    enum State : uint8_t
    {
        Grabbing,
        Free
    };

    struct Commands
    {
        CommandPtr grab;
        CommandPtr ungrab;
    };

    void shouldGrabMouse(bool grab);
    bool shouldGrabMouse() const;

    State state() const;
    void state(State state);

    Camera *camera();

    Commands &commands();

    void registerWith(Engine &) override;
    void registerCommands(CommandProcessor &) override;

    BL_DECLARE_RUNTIME_SIZED(MouseLookPlugin, Impl);
};

} // namespace ge

#endif
