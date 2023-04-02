#ifndef GE_MOUSE_LOOK_PLUGIN_HPP
#define GE_MOUSE_LOOK_PLUGIN_HPP

#include "ge/Camera.hpp"
#include "ge/Plugin.hpp"

namespace ge {

struct GE_API MouseLookPlugin final : public PluginImpl<MouseLookPlugin>
{
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

    MouseLookPlugin();
    ~MouseLookPlugin() override;

    void shouldGrabMouse(bool grab);
    bool shouldGrabMouse() const;

    State state() const;
    void state(State state);

    Camera *camera();

    Commands &commands();

    void registerWith(Engine &) override;
    void registerCommands(CommandProcessor &) override;

private:
    DECLARE_PIMPL(GE_API, self);
};

} // namespace ge

#endif
