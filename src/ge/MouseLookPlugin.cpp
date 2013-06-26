#include "ge/MouseLookPlugin.hpp"
#include "ge/CommandParams.hpp"
#include "ge/GameWindow.hpp"
#include "ge/KeyHandler.hpp"
#include "ge/Engine.hpp"

namespace ge {

struct MouseLookPlugin::Data {
    Data();

    Engine *_engine;
    bool _should_grab;
    State _state;
    Camera *_camera;
    Commands _commands;

    void stateChanged(State new_state);
    void setState(State new_state);

    // commands
    void runGrabMouse(const Event<CommandEvent>&, const Array<CommandArg>&);
    void runUngrabMouse(const Event<CommandEvent>&, const Array<CommandArg>&);

    // handlers
    void handleMouseClick(const Event<MouseButton>&);
    void handleMouseMove(const Event<MouseMoved>&);
    void handleFocusChanged(const Event<FocusChanged>&);
};

MouseLookPlugin::Data::Data() :
    _engine(0),
    _should_grab(true),
    _state(Free),
    _camera(0),
    _commands()
{
    _commands.grab = makeCommand(this, &MouseLookPlugin::Data::runGrabMouse, NULL_PARAMS, "mouseLook.grab", "");
    _commands.ungrab = makeCommand(this, &MouseLookPlugin::Data::runUngrabMouse, NULL_PARAMS, "mouseLook.ungrab", "");
}

void MouseLookPlugin::Data::stateChanged(State new_state) {
    if (_engine == 0)
        return;
    
    GameWindow& win = _engine->window();
    switch (new_state) {
    case Grabbing: {
        if (_camera)
            _camera->mouseLook(true);
        size w, h;
        win.windowSize(w, h);
        win.setMouse(w / 2, h / 2);
        win.showMouseCursor(false);
    } break;
    case Free: {
        win.showMouseCursor(true);
        if (_camera)
            _camera->mouseLook(false);
    } break;
    }
}

void MouseLookPlugin::Data::setState(State new_state) {
    if (_state != new_state) {
        _state = new_state;
        stateChanged(new_state);
    }
}

void MouseLookPlugin::Data::runGrabMouse(const Event<CommandEvent>&, const Array<CommandArg>&) {
    if (_should_grab)
        setState(Grabbing);
}

void MouseLookPlugin::Data::runUngrabMouse(const Event<CommandEvent>&, const Array<CommandArg>&) {
    setState(Free);
}

void MouseLookPlugin::Data::handleMouseClick(const Event<MouseButton>& ev) {
    if (_should_grab && _state == Free) {
        setState(Grabbing);
        ev.abort = true;
    }
}

void MouseLookPlugin::Data::handleMouseMove(const Event<MouseMoved>& ev) {
    GameWindow& win = ev.info.window;
    if (_state == Grabbing) {
        size w, h;
        win.windowSize(w, h);
        win.setMouse(w / 2, h / 2);
    }
}

void MouseLookPlugin::Data::handleFocusChanged(const Event<FocusChanged>& ev) {
    setState(Free);
}

MouseLookPlugin::MouseLookPlugin() :
    self(new Data)
{}

MouseLookPlugin::~MouseLookPlugin() {
    delete self;
}

void MouseLookPlugin::shouldGrabMouse(bool grab) {
    self->_should_grab = grab;
    self->setState(grab ? Grabbing : Free);
}

bool MouseLookPlugin::shouldGrabMouse() const {
    return self->_should_grab;
}

void MouseLookPlugin::state(MouseLookPlugin::State state) {
    self->setState(state);
}

MouseLookPlugin::State MouseLookPlugin::state() const {
    return self->_state;
}

void MouseLookPlugin::camera(Camera *cam) {
    self->_camera = cam;
}

Camera *MouseLookPlugin::camera() {
    return self->_camera;
}

MouseLookPlugin::Commands& MouseLookPlugin::commands() {
    return self->_commands;
}

void MouseLookPlugin::registerWith(Engine& e) {
    self->_engine = &e;
    GameWindow& win = e.window();
    win.events().mouseButton.reg(makeEventHandler(self, &MouseLookPlugin::Data::handleMouseClick));
    win.events().mouseMoved.reg(makeEventHandler(self, &MouseLookPlugin::Data::handleMouseMove));
    win.events().focusChanged.reg(makeEventHandler(self, &MouseLookPlugin::Data::handleFocusChanged));
}

void MouseLookPlugin::registerCommands(CommandProcessor& proc) {
    proc.define(self->_commands.grab);
    proc.define(self->_commands.ungrab);
}

} // namespace ge
