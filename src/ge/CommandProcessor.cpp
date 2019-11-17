#include "ge/CommandProcessor.hpp"

#include "err/err.hpp"
#include "ge/Engine.hpp"
#include "ge/Event.hpp"
#include "ge/Tokenizer.hpp"
#include "sys/fs.hpp"
#include "util/range.hpp"
#include "util/string.hpp"

#include <unordered_map>
#include <vector>

namespace ge {

using CommandMap = std::unordered_map<std::string_view, CommandPtr>;

namespace {
bool
coerceKeyCombo(CommandArg &arg)
{
    UNUSED(arg);
    ERR_ONCE("not yet implemented");
    // TODO: implement
    return false;
}
} // namespace

struct CommandProcessor::Data
{
    Engine &engine;
    CommandMap commands;
    std::vector<std::string> scriptDirs;
    Data(Engine &e) : engine(e) {}
};

DECLARE_PIMPL_DEL(CommandProcessor);

CommandProcessor::CommandProcessor(Engine &e) : self(new Data(e)) {}

CommandProcessor::~CommandProcessor() = default;

std::vector<CommandPtr>
CommandProcessor::commands() const
{
    std::vector<CommandPtr> coms;
    coms.reserve(self->commands.size());
    for (const auto &ent : self->commands)
        coms.push_back(ent.second);
    return coms;
}

Engine &
CommandProcessor::engine()
{
    return self->engine;
}

bool
CommandProcessor::addScriptDirectory(std::string dir, bool check_exists)
{
    for (const auto &scriptDir : self->scriptDirs)
        if (dir == scriptDir)
            return true;

    if (check_exists && !sys::fs::directoryExists(dir))
        return false;

    self->scriptDirs.emplace_back(std::move(dir));
    return true;
}

ArrayView<const std::string>
CommandProcessor::scriptDirectories() const
{
    return view_array(self->scriptDirs);
}

CommandPtr
CommandProcessor::command(std::string_view comname) const
{
    auto it = self->commands.find(comname);
    if (it != self->commands.end())
        return it->second;
    return nullptr;
}

bool
CommandProcessor::define(CommandPtr comm, bool unique)
{
    if (!comm) {
        ERR(engine().out(), "null command");
        return false;
    }

    const auto &comname = comm->name();
    if (comname.empty()) {
        ERR(engine().out(), "cannot define command without a name");
        return false;
    }

    auto it = self->commands.find(comname);
    auto dup = it != self->commands.end();
    if (dup && unique) {
        ERR(engine().out(), string_concat("duplicate command name: ", comname));
        return false;
    }

    if (dup)
        it->second = std::move(comm);
    else
        self->commands[comname] = std::move(comm);
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
    return exec(com, args);
}

bool
CommandProcessor::exec(ArrayView<CommandArg> args)
{
    if (args.size() == 0)
        return true;
    const std::string *com_name = nullptr;
    CommandPtr comm;
    if (args[0].type() == CommandArgType::String) {
        com_name = &args[0].string;
    } else if (args[0].type() == CommandArgType::CommandRef) {
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

    bool ok = exec(comm, { args.data() + 1, args.size() - 1 });
    return ok;
}

bool
CommandProcessor::exec(CommandPtr &com, ArrayView<CommandArg> args)
{
    if (!com) {
        ERR(engine().out(), "Command == 0");
        return false;
    }

    using PT = CommandParamType;
    using AT = CommandArgType;

    const auto &comname = com->name();
    const auto &params = com->parameters();
    bool rest_args = params.size() > 0 && params[params.size() - 1] == PT::List;
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
        if (param != PT::Any) {
            AT val_type = AT::String;
            switch (param.value) {
            case PT::String:
                val_type = AT::String;
                break;
            case PT::Integer:
                val_type = AT::Integer;
                break;
            case PT::Number:
                val_type = AT::Number;
                break;
            case PT::Command:
                val_type = AT::CommandRef;
                break;
            case PT::KeyCombo:
                val_type = AT::KeyCombo;
                break;
            case PT::VarRef:
                val_type = AT::VarRef;
                break;
            case PT::Any:
                UNREACHABLE;
            case PT::List:
                UNREACHABLE;
            }

            // perform some implicit conversion
            if (val_type != args[i].type()) {
                if (val_type == AT::Number && args[i].type() == AT::Integer) {
                    args[i] = CommandArg(double(args[i].integer));
                } else if (val_type == AT::CommandRef &&
                           args[i].type() == AT::String) {
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
                } else if (val_type == AT::KeyCombo &&
                           args[i].type() == AT::String) {
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
                    err << ", expected: " << val_type;
                    err << ", got: " << args[i].type();
                    ERR(engine().out(), err.str());
                    return false;
                }
            } else if (val_type == AT::CommandRef) {
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
        sys::io::stdout() << "loading script: " << file << "\n";
        return loadStream(*opt_stream, file);
    }

not_found:

    sys::io::stdout() << "loading script: " << name << " -> not found"
                      << "\n";

    if (!quiet)
        ERR(engine().out(), string_concat("opening script: ", name));

    return false;
}

bool
CommandProcessor::loadStream(sys::io::InStream &inp, std::string_view inp_name)
{
    bool ok = true;
    std::vector<CommandArg> args;
    ParserState state(inp, inp_name);
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
CommandProcessor::commandParamType(CommandArgType type)
{
    switch (type.value) {
    case CommandArgType::String:
        return CommandParamType::String;
    case CommandArgType::Integer:
        return CommandParamType::Integer;
    case CommandArgType::Number:
        return CommandParamType::Number;
    case CommandArgType::KeyCombo:
        return CommandParamType::KeyCombo;
    case CommandArgType::CommandRef:
        return CommandParamType::Command;
    case CommandArgType::VarRef:
        return CommandParamType::VarRef;
    case CommandArgType::Nil:
        FATAL_ERR("Nil has no declarable type");
    }
    UNREACHABLE;
}

} // namespace ge
