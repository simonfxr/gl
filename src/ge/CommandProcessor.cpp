#include "ge/CommandProcessor.hpp"
#include "ge/Engine.hpp"
#include "ge/Event.hpp"
#include "ge/Tokenizer.hpp"

#include "err/err.hpp"

#include "sys/fs.hpp"

#include <vector>

namespace ge {

#define NULL_COMMAND_REF CommandPtr()

typedef std::map<std::string, CommandPtr> CommandMap;

size
CommandProcessor::size() const
{
    return SIZE(commands.size());
}

bool
CommandProcessor::addScriptDirectory(const std::string &dir, bool check_exists)
{
    for (const auto &scriptDir : scriptDirs)
        if (dir == scriptDir)
            return true;

    sys::fs::ObjectType type = sys::fs::Directory;
    if (check_exists && !sys::fs::exists(dir, &type))
        return false;

    if (type != sys::fs::Directory)
        return false;

    scriptDirs.push_back(dir);
    return true;
}

const std::vector<std::string> &
CommandProcessor::scriptDirectories()
{
    return scriptDirs;
}

CommandPtr
CommandProcessor::command(const std::string &comname)
{
    auto it = commands.find(comname);
    if (it != commands.end())
        return it->second;
    return NULL_COMMAND_REF;
}

bool
CommandProcessor::define(const CommandPtr &comm, bool unique)
{
    return define(comm->name(), comm, unique);
}

bool
CommandProcessor::define(const std::string &comname,
                         const CommandPtr &comm,
                         bool unique)
{
    if (!comm) {
        ERR(engine().out(), (comname + ": null command").c_str());
        return false;
    }

    if (comname.empty()) {
        ERR(engine().out(), "cannot define command without name");
        return false;
    }

    bool dup = command(comname) != nullptr;
    if (dup && unique) {
        ERR(engine().out(), ("duplicate command name: " + comname).c_str());
        return false;
    }

    commands[comname] = comm;
    return dup;
}

bool
CommandProcessor::exec(const std::string &comname, Array<CommandArg> &args)
{
    CommandPtr com = command(comname);
    if (!com) {
        ERR(engine().out(), ("command not found: " + comname).c_str());
        return false;
    }
    return exec(com, args, comname);
}

bool
coerceKeyCombo(CommandArg &arg)
{
    UNUSED(arg);
    ERR_ONCE(ERROR_DEFAULT_STREAM, "not yet implemented");
    // TODO: implement
    return false;
}

bool
CommandProcessor::exec(Array<CommandArg> &args)
{
    if (args.size() == 0)
        return true;
    const std::string *com_name = nullptr;
    CommandPtr comm;
    if (args[0].type == String) {
        com_name = args[0].string;
    } else if (args[0].type == CommandRef) {
        com_name = args[0].command.name;
        comm = *args[0].command.ref;
        if (!comm && !com_name->empty()) {
            comm = command(*com_name);
        }
    } else {
        ERR(engine().out(), "cannot execute command: invalid type");
        return false;
    }

    if (!comm) {
        comm = command(*com_name);
        if (!comm) {
            ERR(engine().out(), "unknown command: " + *com_name);
            return false;
        }
    }

    Array<CommandArg> argsArr = SHARE_ARRAY(args.size() - 1, &args[1]);
    bool ok = exec(comm, argsArr, *com_name);
    return ok;
}

bool
CommandProcessor::exec(CommandPtr &com,
                       Array<CommandArg> &args,
                       const std::string &comname)
{

    if (!com) {
        ERR(engine().out(), "Command == 0");
        return false;
    }

    const Array<CommandParamType> &params = com->parameters();
    bool rest_args =
      params.size() > 0 && params[params.size() - 1] == ListParam;
    defs::size nparams = rest_args ? params.size() - 1 : params.size();

    if (nparams != args.size() && !(rest_args && args.size() > nparams)) {
        sys::io::ByteStream err;
        if (!comname.empty())
            err << "executing Command " << comname << ": ";
        else
            err << "executing unknown Command: ";
        err << "expected " << nparams;
        if (rest_args)
            err << " or more";
        err << " got " << args.size();
        err << " (rest_args=" << rest_args << ", nparams=" << nparams << ")";
        ERR(engine().out(), err.str());
    }

    std::vector<CommandPtr> keepAlive;

    for (index i = 0; i < nparams; ++i) {
        if (params[i] != AnyParam) {

            CommandType val_type = String;
            switch (params[i]) {
            case StringParam:
                val_type = String;
                break;
            case IntegerParam:
                val_type = Integer;
                break;
            case NumberParam:
                val_type = Number;
                break;
            case CommandParam:
                val_type = CommandRef;
                break;
            case KeyComboParam:
                val_type = KeyCombo;
                break;
            case VarRefParam:
                val_type = VarRef;
                break;
            case AnyParam:
                ASSERT_FAIL();
                break;
            case ListParam:
                ASSERT_FAIL();
                break;
            }

            if (val_type != args[i].type) {
                if (val_type == Number && args[i].type == Integer) {
                    args[i].type = Integer;
                    args[i].number = double(args[i].integer);
                } else if (val_type == CommandRef && args[i].type == String) {
                    CommandPtr comArg = command(*args[i].string);
                    if (!comArg) {
                        sys::io::ByteStream err;
                        if (!comname.empty())
                            err << "executing Command " << comname << ": ";
                        else
                            err << "executing unknown Command: ";
                        err << " command not found: " << *args[i].string;
                        ERR(engine().out(), err.str());
                        return false;
                    }
                    keepAlive.push_back(comArg);
                    args[i].type = CommandRef;
                    args[i].command.name = args[i].string;
                    args[i].command.ref = new CommandPtr(comArg);
                } else if (val_type == KeyCombo && args[i].type == String) {
                    if (!coerceKeyCombo(args[i])) {
                        sys::io::ByteStream err;
                        if (!comname.empty())
                            err << "executing Command " << comname << ": ";
                        else
                            err << "executing unknown Command: ";
                        err << " invalid KeyCombo, position: " << (i + 1);
                        ERR(engine().out(), err.str());
                        return false;
                    }
                } else {
                    sys::io::ByteStream err;
                    if (!comname.empty())
                        err << "executing Command " << comname << ": ";
                    else
                        err << "executing unknown Command: ";
                    err << " type mismatch, position: " << (i + 1);
                    err << ", expected: " << prettyCommandType(val_type);
                    err << ", got: " << prettyCommandType(args[i].type);
                    ERR(engine().out(), err.str());
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

bool
CommandProcessor::loadScript(const std::string &name, bool quiet)
{

    std::string file = sys::fs::lookup(scriptDirectories(), name);
    if (file.empty())
        goto not_found;

    {
        sys::io::FileStream filestream(file, "rb");
        if (!filestream.isOpen())
            goto not_found;
        sys::io::stdout() << "loading script: " << file << sys::io::endl;
        return loadStream(filestream, file);
    }

not_found:

    sys::io::stdout() << "loading script: " << name << " -> not found"
                      << sys::io::endl;

    if (!quiet)
        ERR(engine().out(), ("opening script: " + name).c_str());

    return false;
}

bool
CommandProcessor::loadStream(sys::io::InStream &inp,
                             const std::string &inp_name)
{
    bool ok = true;
    std::vector<CommandArg> args;
    ParseState state(inp, inp_name);
    bool done = false;
    while (!done) {
        ok = tokenize(state, args);
        if (!ok) {
            if (state.in_state != sys::io::StreamResult::EOF)
                ERR(engine().out(), "parsing failed: " + state.filename);
            else
                ok = true;
            done = true;
            goto next;
        }

        ok = execCommand(args);
        if (!ok) {
            ERR(engine().out(), "executing command");
            done = true;
            goto next;
        }

    next:;
        for (auto &arg : args)
            arg.free();
        args.clear();
    }

    return ok;
}

bool
CommandProcessor::evalCommand(const std::string &cmd)
{
    sys::io::ByteStream inp(cmd);
    return loadStream(inp, "<eval string>");
}

bool
CommandProcessor::execCommand(std::vector<CommandArg> &args)
{
    if (!args.empty()) {
        Array<CommandArg> com_args = SHARE_ARRAY(SIZE(args.size()), &args[0]);
        return execCommand(com_args);
    }
    Array<CommandArg> com_args = ARRAY_INITIALIZER(CommandArg);
    return execCommand(com_args);
}

bool
CommandProcessor::execCommand(Array<CommandArg> &args)
{
    if (args.size() == 0)
        return true;

    bool ok = exec(args);

    if (!ok) {
        sys::io::ByteStream err;
        err << "executing command failed: ";

        {
            CommandPrettyPrinter printer;
            printer.out(err);
            printer.print(args);
        }

        ERR(engine().out(), err.str());
    }

    return ok;
}

CommandParamType
CommandProcessor::commandParamType(CommandType type)
{
    switch (type) {
    case String:
        return StringParam;
    case Integer:
        return IntegerParam;
    case Number:
        return NumberParam;
    case KeyCombo:
        return KeyComboParam;
    case CommandRef:
        return CommandParam;
    case VarRef:
        return VarRefParam;
    case Nil:
        FATAL_ERR(ERROR_DEFAULT_STREAM, "Nil has no declarable type");
    default:
        ERR(ERROR_DEFAULT_STREAM, "invalid type: returning IntegerParam");
        return IntegerParam;
    }
}

CommandArg
CommandProcessor::cast(const CommandArg &val, CommandType type)
{
    UNUSED(val);
    UNUSED(type);
    FATAL_ERR(ERROR_DEFAULT_STREAM, "not yet implemented");
}

bool
CommandProcessor::isAssignable(CommandParamType lval, CommandType rval)
{
    UNUSED(lval);
    UNUSED(rval);
    FATAL_ERR(ERROR_DEFAULT_STREAM, "not yet implemented");
}

const char *
prettyCommandType(CommandType type)
{
    UNUSED(type);
    return "<type: not yet implemented>";
}

const char *
prettyCommandParamType(CommandParamType type)
{
    UNUSED(type);
    return "<type: not yet implemented>";
}

} // namespace ge
