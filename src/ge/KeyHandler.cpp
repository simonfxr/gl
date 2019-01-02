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
    State states[keycode::Count];
    EventSource<KeyPressed> keyPressedEvent;

    explicit Data(CommandProcessor &proc);
};

DECLARE_PIMPL_DEL(KeyHandler);

KeyHandler::Data::Data(CommandProcessor &proc) : processor(proc)
{
    for (auto &state : states)
        state = State();
}

#define CHECK_KEYCODE(kc)                                                      \
    ASSERT_MSG((kc) >= 0 && (kc) < int32_t(keycode::Count), "invalid keycode")

KeyHandler::KeyHandler(CommandProcessor &proc) : self(new Data(proc)) {}

void
KeyHandler::keyPressed(KeyCode code)
{
    // std::cerr << "key pressed: " << self->frame_id << " " <<
    // prettyKeyCode(code) << std::endl;
    auto idx = int32_t(code);
    CHECK_KEYCODE(idx);
    self->states[idx] = State(true, self->frame_id);
    self->keyPressedEvent.raise(makeEvent(KeyPressed(*this, code)));
}

void
KeyHandler::keyReleased(KeyCode code)
{
    // std::cerr << "key released: " << self->frame_id << " " <<
    // prettyKeyCode(code) << std::endl;
    auto idx = int32_t(code);
    CHECK_KEYCODE(idx);
    if (self->states[idx].down)
        self->states[idx] = State(false, self->frame_id);
}

void
KeyHandler::keyEvent(Key key)
{
    if (key.state == keystate::Pressed)
        keyPressed(key.code);
    else if (key.state == keystate::Released)
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
    auto idx = int32_t(code);
    CHECK_KEYCODE(idx);
    State state = self->states[idx];

    if (state.timestamp == self->frame_id)
        return state.down ? keystate::Pressed : keystate::Released;

    return state.down ? keystate::Down : keystate::Up;
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

// static const char *prettyKeyState(KeyState state) {
//     switch (state) {
//     case Up: return "Up";
//     case Down: return "Down";
//     case Pressed: return "Pressed";
//     case Released: return "Released";
//     default: return "<unknown>";
//     }
// }

void
KeyHandler::handleCommands()
{

    auto it = self->bindings.begin();
    for (; it != self->bindings.end(); ++it) {
        auto &bind = it->first.binding;

        for (size_t i = 0; i < bind->size(); ++i) {

            KeyCode code = bind->at(i).code;
            KeyState reqState = bind->at(i).state;
            KeyState curState = keyState(code);

            bool match = (reqState & curState) == reqState;

            // if (curState != Up)
            //     std::cerr << "checking key: " << prettyKeyCode(code) <<  "
            //     req: " << prettyKeyState(reqState) << " cur: " <<
            //     prettyKeyState(curState) << " -> " << match << std::endl;

            if (!match)
                goto next;
        }

        //        std::cerr << "executing command: " << self->frame_id << " " <<
        //        it->second->name() << std::endl;
        self->processor.exec(it->second,
                             const_cast<Array<CommandArg> &>(NULL_ARGS));

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
