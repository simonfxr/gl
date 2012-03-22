#ifndef GE_KEYHANDLER_HPP
#define GE_KEYHANDLER_HPP

#include "ge/conf.hpp"
#include "data/Ref.hpp"
#include "ge/KeyBinding.hpp"
#include "ge/CommandProcessor.hpp"

namespace ge {

struct KeyHandler;

struct GE_API KeyPressed {
    KeyHandler& handler;
    keycode::KeyCode key;

    KeyPressed(KeyHandler& h, keycode::KeyCode k) :
        handler(h), key(k) {}
};

struct GE_API KeyHandler {

    KeyHandler(CommandProcessor& proc);
    
    ~KeyHandler();
    
    void keyPressed(KeyCode code);
    
    void keyReleased(KeyCode code);

    void clearStates();

    KeyState keyState(KeyCode code);

    void registerBinding(const Ref<KeyBinding>& binding, const Ref<Command>& comm);

    Ref<Command> unregisterBinding(const Ref<KeyBinding>& binding);

    void handleCommands();

    EventSource<KeyPressed>& keyPressedEvent();

    void handleListBindings(const Event<CommandEvent>&);

private:
    struct Data;
    Data * const self;
};

} // namespace ge

#endif

