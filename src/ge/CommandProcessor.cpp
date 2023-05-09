#include "ge/CommandProcessor.hpp"

#include "err/err.hpp"
#include "ge/Engine.hpp"
#include "ge/Event.hpp"
#include "ge/Tokenizer.hpp"
#include "sys/fs.hpp"
#include "util/range.hpp"
#include "util/string.hpp"

#include <bl/vec.hpp>

#include <unordered_map>

using namespace bl::literals;

namespace ge {

using CommandMap = std::unordered_map<bl::string_view, CommandPtr>;

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

struct CommandProcessor::Impl : public CommandProcessor
{
    Engine &engine;
    CommandMap commands;
    bl::vec<bl::string> scriptDirs;
    explicit Impl(Engine &e) : engine(e) {}
};

BL_DEF_RUNTIME_SIZED(CommandProcessor, CommandProcessor::Impl);

CommandProcessor &
CommandProcessor::construct_at(void *p, Engine &e) noexcept
{
    return construct_impl(p, e);
}

bl::vec<CommandPtr>
CommandProcessor::commands() const
{
    bl::vec<CommandPtr> coms;
    coms.reserve(m().commands.size());
    for (const auto &ent : m().commands)
        coms.push_back(ent.second);
    return coms;
}

Engine &
CommandProcessor::engine()
{
    return m().engine;
}

bool
CommandProcessor::addScriptDirectory(bl::string dir, bool check_exists)
{
    for (const auto &scriptDir : m().scriptDirs)
        if (dir == scriptDir)
            return true;

    if (check_exists && !sys::fs::directoryExists(dir))
        return false;

    m().scriptDirs.emplace_back(bl::move(dir));
    return true;
}

bl::span<const bl::string>
CommandProcessor::scriptDirectories() const
{
    return m().scriptDirs;
}

CommandPtr
CommandProcessor::command(bl::string_view comname) const
{
    auto it = m().commands.find(comname);
    if (it != m().commands.end())
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

    auto it = m().commands.find(comname);
    auto dup = it != m().commands.end();
    if (dup && unique) {
        ERR(engine().out(), scat("duplicate command name: "_k, comname));
        return false;
    }

    if (dup)
        it->second = bl::move(comm);
    else
        m().commands[comname] = bl::move(comm);

    ASSERT(std::as_const(m().commands).at(comname));

    return dup;
}

bool
CommandProcessor::exec(bl::string_view comname, bl::span<CommandArg> args)
{
    auto com = command(comname);
    if (!com) {
        ERR(engine().out(), scat("command not found: "_k, comname));
        return false;
    }
    return exec(com, args);
}

bool
CommandProcessor::exec(bl::span<CommandArg> args)
{
    if (args.empty())
        return true;
    const bl::string *com_name = nullptr;
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
            ERR(engine().out(), scat("unknown command: "_sv, *com_name));
            return false;
        }
    }

    auto ok = exec(comm, { args.data() + 1, args.size() - 1 });
    return ok;
}

bool
CommandProcessor::exec(const CommandPtr &com, bl::span<CommandArg> args)
{
    if (!com) {
        ERR(engine().out(), "Command == 0");
        return false;
    }

    using PT = CommandParamType;
    using AT = CommandArgType;

    const auto &comname = com->name();
    const auto &params = com->parameters();
    auto rest_args = !params.empty() && params[params.size() - 1] == PT::List;
    auto nparams = rest_args ? params.size() - 1 : params.size();

    if (!(rest_args ? args.size() >= nparams : args.size() == nparams)) {
        sys::io::ByteStream err;
        if (!comname.empty())
            err << "executing Command "_k << comname << ": "_k;
        else
            err << "executing unknown Command: "_k;
        err << "expected "_k << nparams;
        if (rest_args)
            err << " or more"_k;
        err << " got "_k << args.size();
        err << " parameters (rest_args="_k << rest_args << ", nparams="_k
            << nparams << ")"_k;
        ERR(engine().out(), err.str());
    }

    bl::vec<CommandPtr> keepAlive;
    for (const auto i : irange(params.size() - (rest_args ? 1 : 0))) {
        const auto &param = params[i];
        if (param != PT::Any) {
            AT val_type = AT::String;
            switch (param) {
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
            case PT::List:
                UNREACHABLE;
            }

            // perform some implicit conversion
            if (val_type != args[i].type()) {
                if (val_type == AT::Number && args[i].type() == AT::Integer) {
                    args[i] = CommandArg(static_cast<double>(args[i].integer));
                } else if (val_type == AT::CommandRef &&
                           args[i].type() == AT::String) {
                    CommandPtr comArg = command(args[i].string);
                    if (!comArg) {
                        sys::io::ByteStream err;
                        if (!comname.empty())
                            err << "executing Command "_k << comname << ": "_k;
                        else
                            err << "executing unknown Command: "_k;
                        err << " command not found: "_k << args[i].string;
                        ERR(engine().out(), err.str());
                        return false;
                    }
                    keepAlive.push_back(comArg);
                    args[i] = CommandArg(bl::move(comArg));
                } else if (val_type == AT::KeyCombo &&
                           args[i].type() == AT::String) {
                    if (!coerceKeyCombo(args[i])) {
                        sys::io::ByteStream err;
                        if (!comname.empty())
                            err << "executing Command "_k << comname << ": "_k;
                        else
                            err << "executing unknown Command: "_k;
                        err << " invalid KeyCombo, position: "_k << (i + 1);
                        ERR(engine().out(), err.str());
                        return false;
                    }
                } else {
                    sys::io::ByteStream err;
                    if (!comname.empty())
                        err << "executing Command "_k << comname << ": "_k;
                    else
                        err << "executing unknown Command: "_k;
                    err << " type mismatch, position: "_k << (i + 1);
                    err << ", expected: "_k << val_type;
                    err << ", got: "_k << args[i].type();
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
CommandProcessor::loadScript(bl::string_view name, bool quiet)
{
    auto file =
      sys::fs::lookup(scriptDirectories(), name, sys::fs::ObjectType::File);
    if (file.empty())
        goto not_found;

    {
        auto hres = sys::io::open(file, sys::io::HM_READ);
        if (!hres)
            goto not_found;
        auto stream =
          sys::io::BufferedInStream<sys::io::InStream>{ hres.value() };
        sys::io::stdout() << "loading script: "_k << file << "\n"_k;
        return loadStream(stream, file);
    }

not_found:

    sys::io::stdout() << "loading script: "_k << name << " -> not found"_k
                      << "\n"_k;

    if (!quiet)
        ERR(engine().out(), scat("opening script: "_sv, name));

    return false;
}

bool
CommandProcessor::loadStream(sys::io::InStream inp, bl::string_view inp_name)
{
    auto state = ParserState{ inp, inp_name };
    auto args = bl::vec<CommandArg>{};
    auto ok = true;
    auto done = false;

    while (!done) {
        ok = tokenize(state, args);
        if (!ok) {
            if (state.in_state != sys::io::IOError::EOF)
                ERR(engine().out(),
                    scat("parsing failed: "_sv, state.filename));
            else
                ok = true;
            done = true;
            goto next;
        }

        ok = execCommand(bl::span(args));
        if (!ok) {
            ERR(engine().out(), "executing command"_k);
            done = true;
            goto next;
        }

    next:;
        args.clear();
    }
    return ok;
}

bool
CommandProcessor::evalCommand(bl::string_view cmd)
{
    sys::io::ByteStream inp(cmd);
    return loadStream(inp, "<eval string>"_k);
}

bool
CommandProcessor::execCommand(bl::span<CommandArg> args)
{
    if (args.empty())
        return true;

    auto ok = exec(args);

    if (!ok) {
        sys::io::ByteStream err;
        err << "executing command failed: "_k;

        {
            auto printer = bl::box<CommandPrettyPrinter>{};
            printer->out(err);
            printer->print(args);
        }

        ERR(engine().out(), err.str());
    }

    return ok;
}

CommandParamType
CommandProcessor::commandParamType(CommandArgType type)
{
    switch (type) {
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
