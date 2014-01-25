#ifndef GE_COMMAND_HPP
#define GE_COMMAND_HPP

#include "ge/conf.hpp"
#include "ge/EngineEvents.hpp"
#include "ge/CommandArgs.hpp"

#include "data/Array.hpp"

#include <string>

namespace ge {

extern GE_API const Array<CommandArg> NULL_ARGS;

struct CommandProcessor;
struct QuotationCommand;

struct GE_API CommandEvent : public EngineEvent {
    CommandEvent(Engine& e, CommandProcessor& proc);
    CommandProcessor& processor;
};

struct GE_API Command : public EventHandler<CommandEvent> {
private:
    const Array<CommandParamType>& params;
    std::string namestr;
    std::string descr;
public:
    Command(const Array<CommandParamType>& ps, const std::string name, const std::string& desc);
    virtual ~Command() {}
    const Array<CommandParamType>& parameters() const { return params; }
    std::string name() const;
    void name(const std::string& new_name);
    const std::string& description() const { return descr; }
    std::string interactiveDescription() const;
    virtual void interactive(const Event<CommandEvent>& ev, const Array<CommandArg>&) = 0;
    virtual void handle(const Event<CommandEvent>& ev);
    virtual QuotationCommand *castToQuotation() { return 0; }
};

struct GE_API QuotationCommand : public Command {
    Quotation * const quotation;
    
    QuotationCommand(const std::string& source, int line, int column, const std::string& desc, Quotation *quot);
    ~QuotationCommand();

    virtual void interactive(const Event<CommandEvent>& ev, const Array<CommandArg>&) OVERRIDE;
    QuotationCommand *castToQuotation() { return this; }

private:

    QuotationCommand& operator =(const QuotationCommand&);
    QuotationCommand(const QuotationCommand&);
};

typedef void (*CommandHandler)(const Event<CommandEvent>&);

GE_API Ref<Command> makeCommand(CommandHandler handler, const std::string& name, const std::string& desc);

typedef void (*ListCommandHandler)(const Event<CommandEvent>&, const Array<CommandArg>&);

GE_API Ref<Command> makeListCommand(ListCommandHandler handler, const std::string& name, const std::string& desc);

GE_API Ref<Command> makeCommand(ListCommandHandler handler, const Array<CommandParamType>& params, const std::string& name, const std::string& desc);

template <typename S>
struct StateCommandHandler {
    typedef void (*type)(S, const Event<CommandEvent>&, const Array<CommandArg>&);
};

template <typename S>
Ref<Command> makeCommand(typename StateCommandHandler<S>::type handler, S state, const Array<CommandParamType>& params, const std::string& name, const std::string& desc);

GE_API Ref<Command> makeStringListCommand(ListCommandHandler handler, const std::string& name, const std::string& desc);

template <typename S>
struct StateHandler : public Command {
    typename StateCommandHandler<S>::type handler;
    S state;
    StateHandler(typename StateCommandHandler<S>::type hndlr, S st,
                 const Array<CommandParamType>& params,
                 const std::string& name, const std::string& desc) :
        Command(params, name, desc),
        handler(hndlr), state(st)
        {}
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        handler(state, e, args);
    }
};

template <typename S>
Ref<Command> makeCommand(typename StateCommandHandler<S>::type handler, S state, const Array<CommandParamType>& params, const std::string& name, const std::string& desc) {
    return Ref<Command>(new StateHandler<S>(handler, state, params, name, desc));
}

template <typename T, typename M>
struct MemberFunCommand : public Command {
private:
    T *o;
    M m;
public:
    MemberFunCommand(T *_o, M _m, const Array<CommandParamType>& params,
                     const std::string& name, const std::string& desc) :
        Command(params, name, desc),
        o(_o), m(_m) {}
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        (o->*m)(e, args);
    }
};

template <typename T>
Ref<Command> makeCommand(T *o, void (T::*m)(const Event<CommandEvent>&, const Array<CommandArg>&), const Array<CommandParamType>& params, const std::string& name, const std::string& desc = "") {
    return Ref<Command>(new MemberFunCommand<T, void (T::*)(const Event<CommandEvent>&, const Array<CommandArg>&)>(o, m, params, name, desc));
}

} // namespace ge

#endif
