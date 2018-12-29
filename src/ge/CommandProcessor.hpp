#ifndef GE_COMMAND_PROCESSOR_HPP
#define GE_COMMAND_PROCESSOR_HPP

#include "ge/Command.hpp"
#include "ge/CommandArgs.hpp"

#include "data/Ref.hpp"

#include "sys/io/Stream.hpp"

#include <map>
#include <vector>

namespace ge {

using namespace defs;

struct Engine;

typedef std::map<std::string, Ref<Command>> CommandMap;

struct GE_API CommandProcessor
{
public:
    Engine &_engine;
    CommandMap commands;
    std::vector<std::string> scriptDirs;

public:
    CommandProcessor(Engine &e) : _engine(e) {}
    defs::size size() const;

    bool addScriptDirectory(const std::string &dir, bool check_exists = true);
    const std::vector<std::string> &scriptDirectories();

    Engine &engine() { return _engine; }

    Ref<Command> command(const std::string comname);

    bool define(const Ref<Command> &comm, bool unique = false);

    bool define(const std::string comname,
                const Ref<Command> &comm,
                bool unique = false);

    bool exec(const std::string &comname, Array<CommandArg> &args);

    bool exec(Ref<Command> &com,
              Array<CommandArg> &args,
              const std::string &com_name = "");

    bool exec(Array<CommandArg> &args);

    bool execCommand(std::vector<CommandArg> &args);

    bool execCommand(Array<CommandArg> &args);

    bool loadStream(sys::io::InStream &inp, const std::string &input_name);

    bool loadScript(const std::string &file, bool quiet = false);

    bool evalCommand(const std::string &cmd);

    CommandArg cast(const CommandArg &val, CommandType type);

    static bool isAssignable(CommandParamType lval, CommandType rval);

    static CommandParamType commandParamType(CommandType type);

private:
    CommandProcessor(const CommandProcessor &);
    CommandProcessor &operator=(const CommandProcessor &);
};

GE_API const char *
prettyCommandType(CommandType type);

GE_API const char *
prettyCommandParamType(CommandParamType type);

} // namespace ge

#endif
