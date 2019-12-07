#include "ge/MouseLookPlugin.hpp"
#include "ge/Engine.hpp"
#include "ge/GameWindow.hpp"
#include "ge/KeyHandler.hpp"

namespace ge {

struct MouseLookPlugin::Data
{
    Data();

    Engine *_engine{};
    bool _should_grab{ true };
    State _state{ Free };
    Camera *_camera{};
    Commands _commands;

    void stateChanged(State new_state);
    void setState(State new_state);
};

DECLARE_PIMPL_DEL(MouseLookPlugin)

MouseLookPlugin::Data::Data()
{
    _commands.grab = makeCommand(
      "mouseLook.grab", "", [this](const Event<CommandEvent> & /*unused*/) {
          if (_should_grab)
              setState(Grabbing);
      });

    _commands.ungrab = makeCommand(
      "mouseLook.ungrab", "", [this](const Event<CommandEvent> & /*unused*/) {
          setState(Free);
      });
}

void
MouseLookPlugin::Data::stateChanged(State new_state)
{
    if (_engine == nullptr)
        return;

    GameWindow &win = _engine->window();
    switch (new_state) {
    case Grabbing: {
        if (_camera)
            _camera->mouseLook(true);
        size_t w;

        size_t h;
        win.windowSize(w, h);
        win.setMouse(int16_t(w) / 2, int16_t(h) / 2);
        win.showMouseCursor(false);
    } break;
    case Free: {
        win.showMouseCursor(true);
        if (_camera)
            _camera->mouseLook(false);
    } break;
    }
}

void
MouseLookPlugin::Data::setState(State new_state)
{
    if (_state != new_state) {
        _state = new_state;
        stateChanged(new_state);
    }
}

MouseLookPlugin::MouseLookPlugin() : self(new Data) {}

MouseLookPlugin::~MouseLookPlugin() = default;

void
MouseLookPlugin::shouldGrabMouse(bool grab)
{
    self->_should_grab = grab;
    self->setState(grab ? Grabbing : Free);
}

bool
MouseLookPlugin::shouldGrabMouse() const
{
    return self->_should_grab;
}

void
MouseLookPlugin::state(MouseLookPlugin::State state)
{
    self->setState(state);
}

MouseLookPlugin::State
MouseLookPlugin::state() const
{
    return self->_state;
}

void
MouseLookPlugin::camera(Camera *cam)
{
    self->_camera = cam;
}

Camera *
MouseLookPlugin::camera()
{
    return self->_camera;
}

MouseLookPlugin::Commands &
MouseLookPlugin::commands()
{
    return self->_commands;
}

void
MouseLookPlugin::registerWith(Engine &e)
{
    self->_engine = &e;
    auto &win = e.window();

    win.events().mouseButton.reg([this](const Event<MouseButton> &ev) {
        if (self->_should_grab && self->_state == Free) {
            self->setState(Grabbing);
            ev.abort = true;
        }
    });

    win.events().mouseMoved.reg([this](const Event<MouseMoved> &ev) {
        auto &curwin = ev.info.window;
        if (self->_state == Grabbing) {
            size_t w;

            size_t h;
            curwin.windowSize(w, h);
            curwin.setMouse(int16_t(w / 2), int16_t(h / 2));
        }
    });

    win.events().focusChanged.reg(
      [this](const Event<FocusChanged> & /*unused*/) { self->setState(Free); });
}

void
MouseLookPlugin::registerCommands(CommandProcessor &proc)
{
    proc.define(self->_commands.grab);
    proc.define(self->_commands.ungrab);
}

} // namespace ge
