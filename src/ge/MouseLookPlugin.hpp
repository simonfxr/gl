#ifndef GE_MOUSE_LOOK_PLUGIN_HPP
#define GE_MOUSE_LOOK_PLUGIN_HPP

#include "ge/Camera.hpp"
#include "ge/Plugin.hpp"

namespace ge {

struct GE_API MouseLookPlugin : public Plugin
{

    enum State
    {
        Grabbing,
        Free
    };

    struct Commands
    {
        Ref<Command> grab;
        Ref<Command> ungrab;
    };

    MouseLookPlugin();
    ~MouseLookPlugin();

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
    struct Data;
    Data *const self;

    MouseLookPlugin(const MouseLookPlugin &);
    MouseLookPlugin &operator=(const MouseLookPlugin &);
};

} // namespace ge

#endif
