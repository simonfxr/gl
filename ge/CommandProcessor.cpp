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
    
bool CommandProcessor::define(const std::string comname, const Ref<Command>& comm, bool unique) {
    if (!comm) {
        ERR((comname + ": null command").c_str());
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


bool CommandProcessor::exec(Ref<Command>& com, Array<CommandArg>& args, const std::string& comname) {

    if (!com) {
        ERR("Command == 0");
        return false;
    }

    const Array<CommandParamType> params = com->parameters();
    bool rest_args = params.size() > 0 && params[params.size() - 1] == ListParam;
    uint32 nparams = rest_args ? params.size() - 1 : params.size();

    if (params.size() != args.size() || (rest_args && args.size() < nparams)) {
        std::ostringstream err;
        if (!comname.empty())
            err << "executing Command " << comname << ": ";
        else
            err << "executing unknown Command: ";
        err << "expected " + nparams;
        if (rest_args)
           err << " or more";
        err << "got " << args.size();
        ERR(err.str().c_str());
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
                    args[i].command.ref = comArg.ptr();
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
            }
        }
    }

    com->interactive(makeEvent(CommandEvent(engine())), args);
    return true;
}

const char *prettyCommandType(CommandType type) {
    return "<type: not yet implemented>";
}

const char *prettyCommandParamType(CommandParamType type) {
    return "<type: not yet implemented>";
}

} // namespace ge
