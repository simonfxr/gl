#include <vector>

#include "data/Array.hpp"

#include "ge/Command.hpp"
#include "ge/Engine.hpp"
#include "ge/CommandParams.hpp"

namespace ge {

const Array<CommandArg> NULL_ARGS = ARRAY_INITIALIZER(CommandArg);

std::string Command::name() const {
    if (!namestr.empty())
        return namestr;
    sys::io::ByteStream id;
    const void *self = this;
    id << "<anonymous " << self << ">";
    return id.str();
}

void Command::name(const std::string& new_name) {
    namestr = new_name;
}

CommandEvent::CommandEvent(ge::Engine& e, CommandProcessor& proc) :
    EngineEvent(e), processor(proc)
{}

Command::Command(const Array<CommandParamType>& ps, const std::string name, const std::string& desc) :
    params(ps), namestr(name), descr(desc)
{
    // std::cerr << "creating command: params: " << &ps << " name = " << name << " nparams = " << params.size() << std::endl;
}

void Command::handle(const Event<CommandEvent>& ev) {
    if (params.size() == 0 || params[params.size() - 1] == ListParam) {
        interactive(ev, NULL_ARGS);
    } else {
        ERR("cannot execute command without arguments: " + name());
    }
}

std::string Command::interactiveDescription() const {
    sys::io::ByteStream desc;
    desc << "command " << name() << sys::io::endl
         << "  params:";

    const char *delim = " ";
    for (index i = 0; i < parameters().size(); ++i) {
        desc << delim;
        delim = ", ";
        switch (parameters()[i]) {
        case StringParam: desc << "string"; break;
        case IntegerParam: desc << "int"; break;
        case NumberParam: desc << "num"; break;
        case KeyComboParam: desc << "key"; break;
        case CommandParam: desc << "command"; break;
        case VarRefParam: desc << "var"; break;
        case AnyParam: desc << "?"; break;
        case ListParam: desc << "*"; break;
        }
    }

    if (parameters().size() == 0)
        desc << " none";

    desc << sys::io::endl
         << "  description: " << description() << sys::io::endl;
        
    return desc.str();
}

static std::string describe(const std::string& source, int line, int column, const std::string& desc, Quotation *quot) {
    sys::io::ByteStream str;
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
        const std::vector<CommandArg>& arg_vec = quotation->operator[](ip);
        OwnedArray<CommandArg> args(SIZE(arg_vec.size()), &arg_vec[0]);
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

struct TypedCommand : public Command {
    ListCommandHandler handler;
    TypedCommand(ListCommandHandler hndlr, const Array<CommandParamType>& params, const std::string& name, const std::string& desc) :
        Command(params, name, desc),
        handler(hndlr) {}
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        handler(e, args);
    }
};

Ref<Command> makeCommand(ListCommandHandler handler, const Array<CommandParamType>& params, const std::string& name, const std::string& desc) {
    return Ref<Command>(new TypedCommand(handler, params, name, desc));
}

struct StringListCommand : public Command {
private:
    ListCommandHandler handler;

public:
    StringListCommand(ListCommandHandler hndlr, const std::string& name, const std::string& desc) :
        Command(LIST_PARAMS, name, desc),
        handler(hndlr) {
        // std::cerr << "list command: " << parameters().size() << std::endl;
    }
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        for (index i = 0; i < args.size(); ++i) {
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
        Command(LIST_PARAMS, name, desc), handler(hndlr) {}
    void interactive(const Event<CommandEvent>& e, const Array<CommandArg>& args) {
        handler(e, args);
    }
};

Ref<Command> makeListCommand(ListCommandHandler handler, const std::string& name, const std::string& desc) {
    return Ref<Command>(new ListCommand(handler, name, desc));
}

} // namespace ge
