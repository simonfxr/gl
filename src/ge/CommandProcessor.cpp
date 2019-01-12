#include "ge/CommandProcessor.hpp"

#include "data/range.hpp"
#include "data/string_utils.hpp"
#include "err/err.hpp"
#include "ge/Engine.hpp"
#include "ge/Event.hpp"
#include "ge/Tokenizer.hpp"
#include "sys/fs.hpp"

#include <vector>

namespace ge {

size_t
CommandProcessor::size() const
{
    return commands.size();
}

bool
CommandProcessor::addScriptDirectory(std::string_view dir, bool check_exists)
{
    for (const auto &scriptDir : scriptDirs)
        if (dir == scriptDir)
            return true;

    if (check_exists && !sys::fs::exists(dir, sys::fs::Directory))
        return false;

    scriptDirs.emplace_back(dir);
    return true;
}

const std::vector<std::string> &
CommandProcessor::scriptDirectories()
{
    return scriptDirs;
}

CommandPtr
CommandProcessor::command(std::string_view comname)
{
    auto it = commands.find(comname);
    if (it != commands.end())
        return it->second;
    return nullptr;
}

bool
CommandProcessor::define(const CommandPtr &comm, bool unique)
{
    return define(comm->name(), comm, unique);
}

bool
CommandProcessor::define(std::string_view comname,
                         const CommandPtr &comm,
                         bool unique)
{
    if (!comm) {
        ERR(engine().out(), string_concat(comname, ": null command"));
        return false;
    }

    if (comname.empty()) {
        ERR(engine().out(), "cannot define command without name");
        return false;
    }

    auto it = commands.find(comname);
    auto dup = it != commands.end();
    if (dup && unique) {
        ERR(engine().out(), string_concat("duplicate command name: ", comname));
        return false;
    }

    if (dup)
        it->second = comm;
    else
        commands[std::string(comname)] = comm;
    return dup;
}

bool
CommandProcessor::exec(std::string_view comname, ArrayView<CommandArg> args)
{
    CommandPtr com = command(comname);
    if (!com) {
        ERR(engine().out(), string_concat("command not found: ", comname));
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
CommandProcessor::exec(ArrayView<CommandArg> args)
{
    if (args.size() == 0)
        return true;
    const std::string *com_name = nullptr;
    CommandPtr comm;
    if (args[0].type() == String) {
        com_name = &args[0].string;
    } else if (args[0].type() == CommandRef) {
        com_name = args[0].command.name.get();
        comm = args[0].command.ref;
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

    bool ok = exec(comm, { args.data() + 1, args.size() - 1 }, *com_name);
    return ok;
}

bool
CommandProcessor::exec(CommandPtr &com,
                       ArrayView<CommandArg> args,
                       std::string_view comname)
{

    if (!com) {
        ERR(engine().out(), "Command == 0");
        return false;
    }

    const auto &params = com->parameters();
    bool rest_args =
      params.size() > 0 && params[params.size() - 1] == ListParam;
    size_t nparams = rest_args ? params.size() - 1 : params.size();

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
    for (const auto i : irange(params.size() - (rest_args ? 1 : 0))) {
        const auto &param = params[i];
        if (param != AnyParam) {
            CommandType val_type = String;
            switch (param) {
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
            case ListParam:
                ASSERT_FAIL();
            }

            if (val_type != args[i].type()) {
                if (val_type == Number && args[i].type() == Integer) {
                    args[i] = CommandArg(double(args[i].integer));
                } else if (val_type == CommandRef && args[i].type() == String) {
                    CommandPtr comArg = command(args[i].string);
                    if (!comArg) {
                        sys::io::ByteStream err;
                        if (!comname.empty())
                            err << "executing Command " << comname << ": ";
                        else
                            err << "executing unknown Command: ";
                        err << " command not found: " << args[i].string;
                        ERR(engine().out(), err.str());
                        return false;
                    }
                    keepAlive.push_back(comArg);
                    args[i] = CommandArg(std::move(comArg));
                } else if (val_type == KeyCombo && args[i].type() == String) {
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
                    err << ", got: " << prettyCommandType(args[i].type());
                    ERR(engine().out(), err.str());
                    return false;
                }
            } else if (val_type == CommandRef) {
                if (!args[i].command.ref && !args[i].command.name->empty())
                    args[i].command =
                      CommandValue(command(*args[i].command.name));
            }
        }
    }

    com->interactive(Event(CommandEvent(engine(), *this)), args);
    return true;
}

bool
CommandProcessor::loadScript(std::string_view name, bool quiet)
{

    std::string file = sys::fs::lookup(scriptDirectories(), name);
    if (file.empty())
        goto not_found;

    {
        auto opt_stream = sys::io::HandleStream::open(file, sys::io::HM_READ);
        if (!opt_stream)
            goto not_found;
        sys::io::stdout() << "loading script: " << file << sys::io::endl;
        return loadStream(*opt_stream, file);
    }

not_found:

    sys::io::stdout() << "loading script: " << name << " -> not found"
                      << sys::io::endl;

    if (!quiet)
        ERR(engine().out(), string_concat("opening script: ", name));

    return false;
}

bool
CommandProcessor::loadStream(sys::io::InStream &inp, std::string_view inp_name)
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

        ok = execCommand(view_array(args));
        if (!ok) {
            ERR(engine().out(), "executing command");
            done = true;
            goto next;
        }

    next:;
        args.clear();
    }
    return ok;
}

bool
CommandProcessor::evalCommand(std::string_view cmd)
{
    sys::io::ByteStream inp(cmd);
    return loadStream(inp, "<eval string>");
}

bool
CommandProcessor::execCommand(ArrayView<CommandArg> args)
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
    }
    CASE_UNREACHABLE;
}

// CommandArg
// CommandProcessor::cast(const CommandArg &val, CommandType type)
// {
//     UNUSED(val);
//     UNUSED(type);
//     FATAL_ERR(ERROR_DEFAULT_STREAM, "not yet implemented");
// }

// bool
// CommandProcessor::isAssignable(CommandParamType lval, CommandType rval)
// {
//     UNUSED(lval);
//     UNUSED(rval);
//     FATAL_ERR(ERROR_DEFAULT_STREAM, "not yet implemented");
// }

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
