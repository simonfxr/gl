#ifndef GE_COMMAND_HPP
#define GE_COMMAND_HPP

#include "ge/EngineEvents.hpp"
#include "ge/CommandArgs.hpp"

#include "data/Array.hpp"

#include <string>

namespace ge {

struct CommandEvent : public EngineEvent {
    CommandEvent(Engine& e) : EngineEvent(e) {}
};

struct Command : public EventHandler<CommandEvent> {
private:
    const Array<CommandParamType> params;
    std::string descr;
public:
    Command(const Array<CommandParamType>& ps, const std::string& desc = "") :
        params(ps), descr(desc) {}

    const Array<CommandParamType>& parameters() const { return params; }
    std::string description() const { return descr; }
    std::string interactiveDescription() const;
    virtual void interactive(const Event<CommandEvent>& ev, const Array<CommandArg>&) = 0;
};

extern const Array<CommandParamType> NULL_PARAMS;

extern const Array<CommandArg> NULL_ARGS;

} // namespace ge

#endif
