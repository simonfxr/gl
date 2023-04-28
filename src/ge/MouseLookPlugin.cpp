#include "ge/MouseLookPlugin.hpp"
#include "ge/Engine.hpp"
#include "ge/GameWindow.hpp"
#include "ge/KeyHandler.hpp"

using namespace bl::literals;

namespace ge {

struct MouseLookPlugin::Impl : public MouseLookPlugin
{
    Impl();
    ~Impl() override;

    Engine *_engine{};
    bool _should_grab{ true };
    State _state{ Free };
    Camera *_camera{};
    Commands _commands;

    void stateChanged(State new_state) const;
    void setState(State new_state);
};

MouseLookPlugin::Impl::~Impl() = default;

BL_DEFINE_RUNTIME_SIZED(MouseLookPlugin, MouseLookPlugin::Impl);

MouseLookPlugin::Impl::Impl()
{
    _commands.grab = makeCommand(
      "mouseLook.grab"_k, ""_k, [this](const Event<CommandEvent> & /*unused*/) {
          if (_should_grab)
              setState(Grabbing);
      });

    _commands.ungrab = makeCommand(
      "mouseLook.ungrab"_k,
      ""_k,
      [this](const Event<CommandEvent> & /*unused*/) { setState(Free); });
}

void
MouseLookPlugin::Impl::stateChanged(State new_state) const
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
        win.setMouse(static_cast<int16_t>(w / 2), static_cast<int16_t>(h / 2));
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
MouseLookPlugin::Impl::setState(State new_state)
{
    if (_state != new_state) {
        _state = new_state;
        stateChanged(new_state);
    }
}

MouseLookPlugin &
MouseLookPlugin::construct_at(void *p) noexcept
{
    return construct_impl(p);
}

void
MouseLookPlugin::shouldGrabMouse(bool grab)
{
    m()._should_grab = grab;
    m().setState(grab ? Grabbing : Free);
}

bool
MouseLookPlugin::shouldGrabMouse() const
{
    return m()._should_grab;
}

void
MouseLookPlugin::state(MouseLookPlugin::State state)
{
    m().setState(state);
}

MouseLookPlugin::State
MouseLookPlugin::state() const
{
    return m()._state;
}

Camera *
MouseLookPlugin::camera()
{
    return m()._camera;
}

MouseLookPlugin::Commands &
MouseLookPlugin::commands()
{
    return m()._commands;
}

void
MouseLookPlugin::registerWith(Engine &e)
{
    m()._engine = &e;
    m()._camera = e.pluginSet().findPluginOrAbort<Camera>();
    auto &win = e.window();

    win.events().mouseButton.reg([this](const Event<MouseButton> &ev) {
        if (m()._should_grab && m()._state == Free) {
            m().setState(Grabbing);
            ev.abort = true;
        }
    });

    win.events().mouseMoved.reg([this](const Event<MouseMoved> &ev) {
        auto &curwin = ev.info.window;
        if (m()._state == Grabbing) {
            size_t w;

            size_t h;
            curwin.windowSize(w, h);
            curwin.setMouse(static_cast<int16_t>(w / 2),
                            static_cast<int16_t>(h / 2));
        }
    });

    win.events().focusChanged.reg(
      [this](const Event<FocusChanged> & /*unused*/) { m().setState(Free); });
}

void
MouseLookPlugin::registerCommands(CommandProcessor &proc)
{
    proc.define(m()._commands.grab);
    proc.define(m()._commands.ungrab);
}

} // namespace ge
