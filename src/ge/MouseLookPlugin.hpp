#ifndef GE_MOUSE_LOOK_PLUGIN_HPP
#define GE_MOUSE_LOOK_PLUGIN_HPP

#include "ge/Camera.hpp"
#include "ge/Plugin.hpp"

namespace ge {

struct GE_API MouseLookPlugin : public Plugin
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

    void camera(Camera *);
    Camera *camera();

    Commands &commands();

    void registerWith(Engine &) final override;
    void registerCommands(CommandProcessor &) final override;

private:
    DECLARE_PIMPL(GE_API, self);
};

} // namespace ge

#endif
