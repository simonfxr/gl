#include "ge/KeyHandler.hpp"
#include "ge/Engine.hpp"

#include <cstring>
#include <map>
#include <utility>

namespace ge {

struct State
{
    bool down{ false };
    uint32_t timestamp{ 0xFFFFFFFFul };

    State() = default;
    State(bool dn, uint32_t t) : down(dn), timestamp(t) {}

    bool operator==(const State &ks) const { return timestamp == ks.timestamp; }

    bool operator!=(const State &ks) const { return !(*this == ks); }
};

struct Binding
{
    std::shared_ptr<KeyBinding> binding;

    explicit Binding(std::shared_ptr<KeyBinding> bind)
      : binding(std::move(bind))
    {}

    bool operator<(const Binding &other) const
    {
        return compareKeyBinding(*binding, *other.binding) < 0;
    }

    bool operator==(const Binding &other) const
    {
        return compareKeyBinding(*binding, *other.binding) == 0;
    }

    bool operator!=(const Binding &other) const { return !(*this == other); }
};

using CommandBindings = std::map<Binding, CommandPtr>;

struct KeyHandler::Data
{
    CommandProcessor &processor;
    uint32_t frame_id{ 1 };
    CommandBindings bindings;
    State states[KeyCode::count];
    EventSource<KeyPressed> keyPressedEvent;

    explicit Data(CommandProcessor &proc);
};

DECLARE_PIMPL_DEL(KeyHandler);

KeyHandler::Data::Data(CommandProcessor &proc) : processor(proc)
{
    for (auto &state : states)
        state = State();
}

#define CHECK_KEYCODE(kc) ASSERT_MSG(kc.is_valid(), "invalid keycode")

KeyHandler::KeyHandler(CommandProcessor &proc) : self(new Data(proc)) {}

void
KeyHandler::keyPressed(KeyCode code)
{
    // std::cerr << "key pressed: " << self->frame_id << " " <<
    // prettyKeyCode(code) << std::endl;
    auto idx = code.numeric();
    CHECK_KEYCODE(code);
    self->states[idx] = State(true, self->frame_id);
    self->keyPressedEvent.raise(Event(KeyPressed(*this, code)));
}

void
KeyHandler::keyReleased(KeyCode code)
{
    // std::cerr << "key released: " << self->frame_id << " " <<
    // prettyKeyCode(code) << std::endl;
    auto idx = code.numeric();
    CHECK_KEYCODE(code);
    if (self->states[idx].down)
        self->states[idx] = State(false, self->frame_id);
}

void
KeyHandler::keyEvent(Key key)
{
    if (key.state == KeyState::Pressed)
        keyPressed(key.code);
    else if (key.state == KeyState::Released)
        keyReleased(key.code);
}

void
KeyHandler::clearStates()
{
    memset(self->states, 0, sizeof self->states);
    self->frame_id = 1;
}

KeyState
KeyHandler::keyState(KeyCode code)
{
    auto idx = code.numeric();
    CHECK_KEYCODE(code);
    State state = self->states[idx];

    if (state.timestamp == self->frame_id)
        return state.down ? KeyState::Pressed : KeyState::Released;

    return state.down ? KeyState::Down : KeyState::Up;
}

void
KeyHandler::registerBinding(const std::shared_ptr<KeyBinding> &binding,
                            const CommandPtr &comm)
{
    self->bindings[Binding(binding)] = comm;
}

CommandPtr
KeyHandler::unregisterBinding(const std::shared_ptr<KeyBinding> &binding)
{
    auto it = self->bindings.find(Binding(binding));
    if (it == self->bindings.end())
        return nullptr;

    return it->second;
}

EventSource<KeyPressed> &
KeyHandler::keyPressedEvent()
{
    return self->keyPressedEvent;
}

void
KeyHandler::handleCommands()
{

    auto it = self->bindings.begin();
    for (; it != self->bindings.end(); ++it) {
        auto &bind = it->first.binding;

        for (size_t i = 0; i < bind->size(); ++i) {

            auto key = (*bind)[i];
            KeyCode code = key.code;
            KeyState reqState = key.state;
            KeyState curState = keyState(code);

            bool match = (int(reqState) & int(curState)) == int(reqState);

            // if (curState != Up)
            //     std::cerr << "checking key: " << prettyKeyCode(code) <<  "
            //     req: " << prettyKeyState(reqState) << " cur: " <<
            //     prettyKeyState(curState) << " -> " << match << std::endl;

            if (!match)
                goto next;
        }

        //        std::cerr << "executing command: " << self->frame_id << " " <<
        //        it->second->name() << std::endl;
        self->processor.exec(it->second, {});

    next:;
    }

    ++self->frame_id;
}

void
KeyHandler::handleListBindings(const Event<CommandEvent> &e)
{
    sys::io::OutStream &out = e.info.engine.out();
    auto it = self->bindings.begin();
    for (; it != self->bindings.end(); ++it) {
        auto &bind = it->first.binding;
        out << "  ";
        {
            CommandPrettyPrinter printer;
            printer.out(out);
            printer.print(*bind);
        }
        out << " -> ";
        QuotationCommand *quot = it->second->castToQuotation();
        if (quot == nullptr) {
            out << it->second->name();
            out << sys::io::endl;
        } else {
            out << sys::io::endl;
            {
                CommandPrettyPrinter printer;
                printer.out(out);
                printer.print(*quot->quotation);
            }
            out << sys::io::endl;
        }
    }
}

} // namespace ge
