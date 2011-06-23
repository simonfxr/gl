#ifndef GE_KEYHANDLER_HPP
#define GE_KEYHANDLER_HPP

#include "defs.h"

#include "data/Ref.hpp"

#include "ge/KeyBinding.hpp"
#include "ge/CommandProcessor.hpp"

namespace ge {

struct KeyHandler {

    KeyHandler(CommandProcessor& proc);
    
    ~KeyHandler();
    
    void keyPressed(KeyCode code);
    
    void keyReleased(KeyCode code);

    KeyState keyState(KeyCode code);

    void registerBinding(const Ref<KeyBinding>& binding, const Ref<Command>& comm);

    Ref<Command> unregisterBinding(const Ref<KeyBinding>& binding);

    void handleCommands();

private:
    struct Data;
    Data * const self;
};

} // namespace ge

#endif

