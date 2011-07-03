#include "ge/CommandProcessor.hpp"
#include "ge/Event.hpp"

#include "error/error.hpp"

#include <vector>
#include <sstream>

namespace ge {

const Ref<Command> NULL_COMMAND_REF;

typedef std::map<std::string, Ref<Command> > CommandMap;
    
uint32 CommandProcessor::size() const {
    return commands.size();
}
    
Ref<Command> CommandProcessor::command(const std::string comname) {
    CommandMap::iterator it = commands.find(comname);
    if (it != commands.end())
        return it->second;
    return NULL_COMMAND_REF;
}

bool CommandProcessor::define(const Ref<Command>& comm, bool unique) {
    return define(comm->name(), comm, unique);
}
    
bool CommandProcessor::define(const std::string comname, const Ref<Command>& comm, bool unique) {
    if (!comm) {
        ERR((comname + ": null command").c_str());
        return false;
    }

    if (comname.empty()) {
        ERR("cannot define command without name");
        return false;
    }
    
    bool dup = command(comname);
    if (dup && unique) {
        ERR(("duplicate command name: " + comname).c_str());
        return false;
    }
    
    commands[comname] = comm;
    return dup;
}

bool CommandProcessor::exec(const std::string& comname, Array<CommandArg>& args) {
    Ref<Command> com = command(comname);
    if (!com) {
        ERR(("command not found: " + comname).c_str());
        return false;
    }
    return exec(com, args, comname);
}

bool coerceKeyCombo(CommandArg& arg) {
    UNUSED(arg);
    ERR_ONCE("not yet implemented");
    // TODO: implement
    return false;
}

bool CommandProcessor::exec(Array<CommandArg>& args) {
    if (args.size() == 0)
        return true;
    const std::string *com_name = 0;
    Ref<Command> comm;
    if (args[0].type == String) {
        com_name = args[0].string;
    } else if (args[0].type == CommandRef) {
        com_name = args[0].command.name;
        comm = *args[0].command.ref;
        if (!comm && com_name->empty()) {
            comm = command(*com_name);
        }
    } else {
        ERR("cannot execute command: invalid type");
        return false;
    }

    if (!comm) {
        comm = command(*com_name);
        if (!comm) {
            ERR("unknown command: " + *com_name);
            return false;
        }
    }

    Array<CommandArg> argsArr(&args[1], args.size() - 1);
    bool ok = exec(comm, argsArr, *com_name);
    return ok;
}

bool CommandProcessor::exec(Ref<Command>& com, Array<CommandArg>& args, const std::string& comname) {

    if (!com) {
        ERR("Command == 0");
        return false;
    }

    const Array<CommandParamType>& params = com->parameters();
    bool rest_args = params.size() > 0 && params[params.size() - 1] == ListParam;
    uint32 nparams = rest_args ? params.size() - 1 : params.size();

    if (nparams != args.size() && !(rest_args && args.size() > nparams)) {
        std::ostringstream err;
        if (!comname.empty())
            err << "executing Command " << comname << ": ";
        else
            err << "executing unknown Command: ";
        err << "expected " << nparams;
        if (rest_args)
           err << " or more";
        err << " got " << args.size();
        ERR(err.str());
    }

    std::vector<Ref<Command> > keepAlive;
    
    for (uint32 i = 0; i < nparams; ++i) {
        if (params[i] != AnyParam) {

            CommandType val_type = String;
            switch (params[i]) {
            case StringParam: val_type = String; break;
            case IntegerParam: val_type = Integer; break;
            case NumberParam: val_type = Number; break;
            case CommandParam: val_type = CommandRef; break;
            case KeyComboParam: val_type = KeyCombo; break;
            }

            if (val_type != args[i].type) {
                if (val_type == Number && args[i].type == Integer) {
                    args[i].type = Integer;
                    args[i].number = args[i].integer;
                } else if (val_type == CommandRef && args[i].type == String) {
                    Ref<Command> comArg = command(*args[i].string);
                    if (!comArg) {
                        std::ostringstream err;
                        if (!comname.empty())
                            err << "executing Command " << comname << ": ";
                        else
                            err << "executing unknown Command: ";
                        err << " command not found: " << *args[i].string;
                        ERR(err.str().c_str());
                        return false;
                    }
                    keepAlive.push_back(comArg);
                    args[i].type = CommandRef;
                    args[i].command.name = args[i].string;
                    args[i].command.ref = new Ref<Command>(comArg);
                } else if (val_type == KeyCombo && args[i].type == String) {
                    if (!coerceKeyCombo(args[i])) {
                        std::ostringstream err;
                        if (!comname.empty())
                            err << "executing Command " << comname << ": ";
                        else
                            err << "executing unknown Command: ";
                        err << " invalid KeyCombo, position: " << (i + 1);
                        ERR(err.str().c_str());
                        return false;
                    }
                } else {
                    std::ostringstream err;
                    if (!comname.empty())
                        err << "executing Command " << comname << ": ";
                    else
                        err << "executing unknown Command: ";
                    err << " type mismatch, position: " << (i + 1);
                    err << ", expected: " << prettyCommandType(val_type);
                    err << ", got: " << prettyCommandType(args[i].type);
                    ERR(err.str().c_str());
                    return false;
                }
            } else if (val_type == CommandRef) {
                if (!*args[i].command.ref && !args[i].command.name->empty()) {
                    *args[i].command.ref = command(*args[i].command.name);
                }
            }
        }
    }

    com->interactive(makeEvent(CommandEvent(engine(), *this)), args);
    return true;
}

CommandParamType CommandProcessor::commandParamType(CommandType type) {
    switch (type) {
    case String: return StringParam;
    case Integer: return IntegerParam;
    case Number: return NumberParam;
    case KeyCombo: return KeyComboParam;
    case CommandRef: return CommandParam;
    case VarRef: return VarRefParam;
    case Nil: ERR("Nil has no declarable type");
    default:
        ERR("invalid type: returning IntegerParam");
        return IntegerParam;
    }
}

CommandArg CommandProcessor::cast(const CommandArg& val, CommandType type) {
    FATAL_ERR("not yet implemented");
}

bool CommandProcessor::isAssignable(CommandParamType lval, CommandType rval) {
    FATAL_ERR("not yet implemented");
}       

const char *prettyCommandType(CommandType type) {
    UNUSED(type);
    return "<type: not yet implemented>";
}

const char *prettyCommandParamType(CommandParamType type) {
    UNUSED(type);
    return "<type: not yet implemented>";
}

} // namespace ge
