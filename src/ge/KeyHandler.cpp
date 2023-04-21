#include "ge/KeyHandler.hpp"
#include "ge/Engine.hpp"

#include <array>
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

    friend std::weak_ordering operator<=>(const State &lhs,
                                          const State &rhs) noexcept
    {
        return lhs.timestamp <=> rhs.timestamp;
    }
};

struct Binding
{
    std::shared_ptr<KeyBinding> binding;

    explicit Binding(std::shared_ptr<KeyBinding> bind) : binding(bl::move(bind))
    {}

    friend auto operator<=>(const Binding &lhs, const Binding &rhs) noexcept
    {
        return *lhs.binding <=> *rhs.binding;
    }
};

using CommandBindings = std::map<Binding, CommandPtr>;

struct KeyHandler::Impl : public KeyHandler
{
    CommandProcessor &processor;
    uint32_t frame_id{ 1 };
    CommandBindings bindings;
    std::array<State, bl::enum_count_of<KeyCode>()> states;
    EventSource<KeyPressed> keyPressedEvent;

    explicit Impl(CommandProcessor &proc);
};

BL_DEFINE_RUNTIME_SIZED(KeyHandler, KeyHandler::Impl);

KeyHandler::Impl::Impl(CommandProcessor &proc) : processor(proc)
{
    for (auto &state : states)
        state = State();
}

KeyHandler &
KeyHandler::construct_at(void *p, CommandProcessor &proc) noexcept
{
    return construct_impl(p, proc);
}

void
KeyHandler::keyPressed(KeyCode code)
{
    // std::cerr << "key pressed: " << m().frame_id << " " <<
    // prettyKeyCode(code) << std::endl;
    auto idx = static_cast<size_t>(code);
    m().states[idx] = State(true, m().frame_id);
    m().keyPressedEvent.raise(Event(KeyPressed(*this, code)));
}

void
KeyHandler::keyReleased(KeyCode code)
{
    // std::cerr << "key released: " << m().frame_id << " " <<
    // prettyKeyCode(code) << std::endl;
    auto idx = static_cast<size_t>(code);
    if (m().states[idx].down)
        m().states[idx] = State(false, m().frame_id);
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
    std::fill(m().states.begin(), m().states.end(), State{});
    m().frame_id = 1;
}

KeyState
KeyHandler::keyState(KeyCode code)
{
    auto idx = static_cast<size_t>(code);
    auto state = m().states[idx];

    if (state.timestamp == m().frame_id)
        return state.down ? KeyState::Pressed : KeyState::Released;

    return state.down ? KeyState::Down : KeyState::Up;
}

void
KeyHandler::registerBinding(const std::shared_ptr<KeyBinding> &binding,
                            const CommandPtr &comm)
{
    m().bindings[Binding(binding)] = comm;
}

CommandPtr
KeyHandler::unregisterBinding(const std::shared_ptr<KeyBinding> &binding)
{
    auto it = m().bindings.find(Binding(binding));
    if (it == m().bindings.end())
        return nullptr;

    return it->second;
}

EventSource<KeyPressed> &
KeyHandler::keyPressedEvent()
{
    return m().keyPressedEvent;
}

void
KeyHandler::handleCommands()
{

    auto it = m().bindings.begin();
    for (; it != m().bindings.end(); ++it) {
        auto &bind = it->first.binding;

        for (size_t i = 0; i < bind->size(); ++i) {

            auto key = (*bind)[i];
            auto code = key.code;
            auto reqState = key.state;
            auto curState = keyState(code);

            auto match =
              (static_cast<int>(reqState) & static_cast<int>(curState)) ==
              static_cast<int>(reqState);

            // if (curState != Up)
            //     std::cerr << "checking key: " << prettyKeyCode(code) <<  "
            //     req: " << prettyKeyState(reqState) << " cur: " <<
            //     prettyKeyState(curState) << " -> " << match << std::endl;

            if (!match)
                goto next;
        }

        //        std::cerr << "executing command: " << m().frame_id << " " <<
        //        it->second->name() << std::endl;
        m().processor.exec(it->second, {});

    next:;
    }

    ++m().frame_id;
}

void
KeyHandler::handleListBindings(const Event<CommandEvent> &e)
{
    sys::io::OutStream &out = e.info.engine.out();
    auto it = m().bindings.begin();
    for (; it != m().bindings.end(); ++it) {
        auto &bind = it->first.binding;
        out << "  ";
        {
            auto printer = bl::box<CommandPrettyPrinter>{};
            printer->out(out);
            printer->print(*bind);
        }
        out << " -> ";
        QuotationCommand *quot = it->second->castToQuotation();
        if (quot == nullptr) {
            out << it->second->name();
            out << "\n";
        } else {
            out << "\n";
            {
                auto printer = bl::box<CommandPrettyPrinter>{};
                printer->out(out);
                printer->print(*quot->quotation);
            }
            out << "\n";
        }
    }
}

} // namespace ge
