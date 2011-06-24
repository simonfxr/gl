#include "ge/Command.hpp"
#include "data/Array.hpp"
#include "ge/Engine.hpp"

#include <sstream>
#include <vector>
#include <iostream>

namespace ge {

const Array<CommandParamType> NULL_PARAMS(0, 0);

const Array<CommandArg> NULL_ARGS(0, 0);

std::string Command::name() const {
    if (!namestr.empty())
        return namestr;
    std::ostringstream id;
    id << "<anonymous " << this << ">";
    return id.str();
}

Command::Command(const Array<CommandParamType>& ps, const std::string name, const std::string& desc) :
    params(ps), namestr(name), descr(desc)
{
//    std::cerr << "creating command: name = " << name << " nparams = " << params.size() << std::endl;
}


void Command::handle(const Event<CommandEvent>& ev) {
    if (params.size() == 0 || params[params.size() - 1] == ListParam)
        interactive(ev, NULL_ARGS);
    else
        ERR("cannot execute command without arguments: " + name());
}

std::string Command::interactiveDescription() const {
    return description();
}

static std::string describe(const std::string& source, int line, int column, const std::string& desc, Quotation *quot) {
    std::ostringstream str;
    str << "<quotation " << source << "@" << line << ":" << column;
    if (!desc.empty())
        str << " " << desc << ">";
    // TODO: extend description with source code of quotation
    UNUSED(quot);
    return str.str();
}

QuotationCommand::QuotationCommand(const std::string& source, int line, int column, const std::string& desc, Quotation *quot) :
    Command(NULL_PARAMS, "<anonymous quotation>", describe(source, line, column, desc, quot)),
    quotation(quot)
{}

QuotationCommand::~QuotationCommand() {
    Quotation& q = *quotation;
    for (uint32 i = 0; i < q.size(); ++i)
        for (uint32 j = 0; j < q[i].size(); ++j)
            q[i][j].free();
    delete quotation;
}

void QuotationCommand::interactive(const Event<CommandEvent>& ev, const Array<CommandArg>&) {
    for (uint32 ip = 0; ip < quotation->size(); ++ip) {
        std::vector<CommandArg>& arg_vec = quotation->operator[](ip);
        Array<CommandArg> args(&arg_vec[0], arg_vec.size());
        ev.info.engine.commandProcessor().exec(args);
    }
}

struct SimpleCommand : public Command {
private:
    CommandHandler handler;
public:
    SimpleCommand(CommandHandler hndlr, const std::string& name, const std::string& desc) :
        Command(NULL_PARAMS, name, desc), handler(hndlr) {}
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>&) {
        handler(e);
    }
};

Ref<Command> makeCommand(CommandHandler handler, const std::string& name, const std::string& desc) {
    return Ref<Command>(new SimpleCommand(handler, name, desc));
}

struct StringListCommand : public Command {
private:
    ListCommandHandler handler;

public:
    StringListCommand(ListCommandHandler hndlr, const std::string& name, const std::string& desc) :
        Command(CONST_ARRAY(CommandParamType, ListParam), name, desc),
        handler(hndlr) {
//        std::cerr << "list command: " << parameters().size() << std::endl;

    }
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        for (uint32 i = 0; i < args.size(); ++i) {
            if (args[i].type != String) {
                ERR("expected String argument");
                return;
            }
        }
        
        handler(e, args);
    }
};

Ref<Command> makeStringListCommand(ListCommandHandler handler, const std::string& name, const std::string& desc) {
    return Ref<Command>(new StringListCommand(handler, name, desc));
}

struct ListCommand : public Command {
private:
    ListCommandHandler handler;
public:
    ListCommand(ListCommandHandler hndlr, const std::string& name, const std::string& desc) :
        Command(CONST_ARRAY(CommandParamType, ListParam), name, desc), handler(hndlr) {}
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        handler(e, args);
    }
};


Ref<Command> makeListCommand(ListCommandHandler handler, const std::string& name, const std::string& desc) {
    return Ref<Command>(new ListCommand(handler, name, desc));
}



} // namespace ge
