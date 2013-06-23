#include "ge/MouseLookPlugin.hpp"
#include "ge/CommandParams.hpp"
#include "ge/GameWindow.hpp"
#include "ge/KeyHandler.hpp"
#include "ge/Engine.hpp"

namespace ge {

struct MouseLookPlugin::Data {
    Data();

    Engine *engine;
    bool should_grab;
    bool grabbed;
    
    Ref<Command> cmdGrabMouse;
    Ref<Command> cmdUngrabMouse;

    void grabChanged(bool _grabbed);
    void setGrab(bool grab);

    // commands
    void runGrabMouse(const Event<CommandEvent>&, const Array<CommandArg>&);
    void runUngrabMouse(const Event<CommandEvent>&, const Array<CommandArg>&);

    // handlers
    void handleMouseClick(const Event<MouseButton>&);
    void handleMouseMove(const Event<MouseMoved>&);
    void handleFocusChanged(const Event<FocusChanged>&);
};

MouseLookPlugin::Data::Data() :
    engine(0),
    should_grab(true),
    grabbed(false),
    cmdGrabMouse(makeCommand(this, &MouseLookPlugin::Data::runGrabMouse, NULL_PARAMS, "grabMouse", "")),
    cmdUngrabMouse(makeCommand(this, &MouseLookPlugin::Data::runUngrabMouse, NULL_PARAMS, "ungrabMouse", ""))
{}

void MouseLookPlugin::Data::grabChanged(bool _grabbed) {
    grabbed = _grabbed;
    GameWindow& win = engine->window();
    if (_grabbed) {
        size w, h;
        win.windowSize(w, h);
        win.setMouse(w / 2, h / 2);
        win.showMouseCursor(false);
    } else {
        win.showMouseCursor(true);
    }
}

void MouseLookPlugin::Data::setGrab(bool grab) {
    if (should_grab != grab) {
        should_grab = grab;
        if (engine)
            grabChanged(should_grab);
    }
}

void MouseLookPlugin::Data::runGrabMouse(const Event<CommandEvent>&, const Array<CommandArg>&) {
    setGrab(true);
}

void MouseLookPlugin::Data::runUngrabMouse(const Event<CommandEvent>&, const Array<CommandArg>&) {
    setGrab(false);
}

void MouseLookPlugin::Data::handleMouseClick(const Event<MouseButton>& ev) {
    if (!grabbed && should_grab) {
        grabChanged(true);
        ev.abort = true;
    }
}

void MouseLookPlugin::Data::handleMouseMove(const Event<MouseMoved>& ev) {
    GameWindow& win = ev.info.window;
    if (grabbed) {
        size w, h;
        win.windowSize(w, h);
        win.setMouse(w / 2, h / 2);
    }
}

void MouseLookPlugin::Data::handleFocusChanged(const Event<FocusChanged>& ev) {
    INFO(std::string("focused? ") + (ev.info.focused ? "yes" : "no"));
    grabChanged(ev.info.focused);
}

MouseLookPlugin::MouseLookPlugin() :
    self(new Data)
{}

MouseLookPlugin::~MouseLookPlugin() {
    delete self;
}

void MouseLookPlugin::grabMouse(bool grab) {
    self->setGrab(grab);
}

bool MouseLookPlugin::grabMouse() const {
    return self->should_grab;
}

void MouseLookPlugin::registerWith(Engine& e) {
    self->engine = &e;
    self->grabChanged(self->should_grab);
    GameWindow& win = e.window();
    win.events().mouseButton.reg(makeEventHandler(self, &MouseLookPlugin::Data::handleMouseClick));
    win.events().mouseMoved.reg(makeEventHandler(self, &MouseLookPlugin::Data::handleMouseMove));
    win.events().focusChanged.reg(makeEventHandler(self, &MouseLookPlugin::Data::handleFocusChanged));
}

void MouseLookPlugin::registerCommands(CommandProcessor& proc) {
    proc.define(self->cmdGrabMouse);
    proc.define(self->cmdUngrabMouse);
}

} // namespace ge
