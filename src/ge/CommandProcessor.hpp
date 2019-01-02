#ifndef GE_COMMAND_PROCESSOR_HPP
#define GE_COMMAND_PROCESSOR_HPP

#include "ge/Command.hpp"
#include "ge/CommandArgs.hpp"

#include "sys/io/Stream.hpp"

#include <map>
#include <vector>

namespace ge {

struct Engine;

typedef std::map<std::string, CommandPtr> CommandMap;

struct GE_API CommandProcessor
{
public:
    Engine &_engine;
    CommandMap commands;
    std::vector<std::string> scriptDirs;

public:
    CommandProcessor(Engine &e) : _engine(e) {}
    defs::size_t size_t() const;

    bool addScriptDirectory(const std::string &dir, bool check_exists = true);
    const std::vector<std::string> &scriptDirectories();

    Engine &engine() { return _engine; }

    CommandPtr command(const std::string &comname);

    bool define(const CommandPtr &comm, bool unique = false);

    bool define(const std::string &comname,
                const CommandPtr &comm,
                bool unique = false);

    bool exec(const std::string &comname, Array<CommandArg> &args);

    bool exec(CommandPtr &com,
              Array<CommandArg> &args,
              const std::string &comname = "");

    bool exec(Array<CommandArg> &args);

    bool execCommand(std::vector<CommandArg> &args);

    bool execCommand(Array<CommandArg> &args);

    bool loadStream(sys::io::InStream &inp, const std::string &inp_name);

    bool loadScript(const std::string &name, bool quiet = false);

    bool evalCommand(const std::string &cmd);

    CommandArg cast(const CommandArg &val, CommandType type);

    static bool isAssignable(CommandParamType lval, CommandType rval);

    static CommandParamType commandParamType(CommandType type);

    CommandProcessor(const CommandProcessor &) = delete;
    CommandProcessor &operator=(const CommandProcessor &) = delete;
};

GE_API const char *
prettyCommandType(CommandType type);

GE_API const char *
prettyCommandParamType(CommandParamType type);

} // namespace ge

#endif
