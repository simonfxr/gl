#include "ge/KeyHandler.hpp"

#include <map>

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

    Data(CommandProcessor& proc);
};

static Ref<Command> NULL_COMMAND(0);

KeyHandler::Data::Data(CommandProcessor& proc) :
    processor(proc), frame_id(0)
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
    int32 idx = int32(code);
    CHECK_KEYCODE(idx);
    self->states[idx] = State(true, self->frame_id);
}
    
void KeyHandler::keyReleased(KeyCode code) {
    int32 idx = int32(code);
    CHECK_KEYCODE(idx);
    self->states[idx] = State(false, self->frame_id);
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

void KeyHandler::handleCommands() {

    CommandBindings::iterator it = self->bindings.begin();
    for (; it != self->bindings.end(); ++it) {
        const Ref<KeyBinding>& bind = it->first.binding;
        
        for (uint32 i = 0; i < bind->size(); ++i)
            if (bind->get(i).state != keyState(bind->get(i).code))
                goto next;

        self->processor.exec(it->second, const_cast<Array<CommandArg>& >(NULL_ARGS));

    next:;
    }
    
    ++self->frame_id;
}

} // namespace ge

