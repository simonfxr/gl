#include "ge/KeyHandler.hpp"

#include <map>
#include <cstring>

namespace ge {

struct State {
    bool down;
    uint32 timestamp;

    State() : down(false), timestamp(0xFFFFFFFFul) {}
    
    State(bool dn, uint32 t) : down(dn), timestamp(t) {}

    bool operator ==(const State& ks) const {
        return timestamp == ks.timestamp;
    }

    bool operator !=(const State& ks) const {
        return !(*this == ks);
    }
};

struct Binding {
    Ref<KeyBinding> binding;

    Binding(const Ref<KeyBinding>& bind) :
        binding(bind) {}

    bool operator <(const Binding& other) const {
        return compareKeyBinding(*binding, *other.binding) < 0;
    }

    bool operator ==(const Binding& other) const {
        return compareKeyBinding(*binding, *other.binding) == 0;
    }

    bool operator !=(const Binding& other) const {
        return !(*this == other);
    }
};

typedef std::map<Binding, Ref<Command> > CommandBindings;

struct KeyHandler::Data {
    CommandProcessor& processor;
    uint32 frame_id;
    CommandBindings bindings;
    State states[keycode::Count];
    EventSource<KeyPressed> keyPressedEvent;

    Data(CommandProcessor& proc);
};

static Ref<Command> NULL_COMMAND(0);

KeyHandler::Data::Data(CommandProcessor& proc) :
    processor(proc), frame_id(1)
{
    for (uint32 i = 0; i < ARRAY_LENGTH(states); ++i)
        states[i] = State();
}

#define CHECK_KEYCODE(kc) ASSERT_MSG((kc) >= 0 && (kc) < int32(keycode::Count), "invalid keycode")

KeyHandler::KeyHandler(CommandProcessor& proc) :
    self(new Data(proc))
{}

KeyHandler::~KeyHandler() { delete self; }

void KeyHandler::keyPressed(KeyCode code) {
    // std::cerr << "key pressed: " << self->frame_id << " " << prettyKeyCode(code) << std::endl;
    int32 idx = int32(code);
    CHECK_KEYCODE(idx);
    self->states[idx] = State(true, self->frame_id);
    self->keyPressedEvent.raise(makeEvent(KeyPressed(*this, code)));
}
    
void KeyHandler::keyReleased(KeyCode code) {
    // std::cerr << "key released: " << self->frame_id << " " << prettyKeyCode(code) << std::endl;
    int32 idx = int32(code);
    CHECK_KEYCODE(idx);
    if (self->states[idx].down)
        self->states[idx] = State(false, self->frame_id);
}

void KeyHandler::clearStates() {
    memset(self->states, 0, sizeof self->states);
    self->frame_id = 1;
}

KeyState KeyHandler::keyState(KeyCode code) {
    int32 idx = int32(code);
    CHECK_KEYCODE(idx);
    State state = self->states[idx];

    if (state.timestamp == self->frame_id)
        return state.down ? Pressed : Released;
    else
        return state.down ? Down : Up;
}

void KeyHandler::registerBinding(const Ref<KeyBinding>& binding, const Ref<Command>& comm) {
    self->bindings[Binding(binding)] = comm;
}

Ref<Command> KeyHandler::unregisterBinding(const Ref<KeyBinding>& binding) {
    CommandBindings::iterator it = self->bindings.find(Binding(binding));
    if (it == self->bindings.end())
        return NULL_COMMAND;
    else
        return it->second;
}

EventSource<KeyPressed>& KeyHandler::keyPressedEvent() {
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

void KeyHandler::handleCommands() {

    CommandBindings::iterator it = self->bindings.begin();
    for (; it != self->bindings.end(); ++it) {
        const Ref<KeyBinding>& bind = it->first.binding;

        for (defs::index i = 0; i < bind->size(); ++i) {

            KeyCode code = bind->at(i).code;
            KeyState reqState = bind->at(i).state;
            KeyState curState = keyState(code);

            bool match = (reqState & curState) == reqState;

             // if (curState != Up)
             //     std::cerr << "checking key: " << prettyKeyCode(code) <<  " req: " << prettyKeyState(reqState) << " cur: " << prettyKeyState(curState) << " -> " << match << std::endl;

            if (!match)
                goto next;
        }

//        std::cerr << "executing command: " << self->frame_id << " " << it->second->name() << std::endl;
        self->processor.exec(it->second, const_cast<Array<CommandArg>& >(NULL_ARGS));

    next:;
    }
    
    ++self->frame_id;
}

} // namespace ge

