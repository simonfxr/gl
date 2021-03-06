#ifndef GE_KEYHANDLER_HPP
#define GE_KEYHANDLER_HPP

#include "ge/CommandProcessor.hpp"
#include "ge/KeyBinding.hpp"
#include "ge/conf.hpp"

#include <memory>

namespace ge {

struct KeyHandler;

struct GE_API KeyPressed
{
    KeyHandler &handler;
    KeyCode key;

    KeyPressed(KeyHandler &h, KeyCode k) : handler(h), key(k) {}
};

struct GE_API KeyHandler
{
    explicit KeyHandler(CommandProcessor &proc);

    void keyPressed(KeyCode code);

    void keyReleased(KeyCode code);

    void keyEvent(Key key);

    void clearStates();

    KeyState keyState(KeyCode code);

    void registerBinding(const std::shared_ptr<KeyBinding> &binding,
                         const CommandPtr &comm);

    CommandPtr unregisterBinding(const std::shared_ptr<KeyBinding> &binding);

    void handleCommands();

    EventSource<KeyPressed> &keyPressedEvent();

    void handleListBindings(const Event<CommandEvent> &);

private:
    DECLARE_PIMPL(GE_API, self);
};

} // namespace ge

#endif
