#ifndef GE_COMMAND_PROCESSOR_HPP
#define GE_COMMAND_PROCESSOR_HPP

#include "ge/Command.hpp"
#include "ge/CommandArgs.hpp"
#include "sys/io/Stream.hpp"

#include <map>
#include <string_view>
#include <vector>

namespace ge {

struct Engine;

typedef std::map<std::string, CommandPtr, std::less<>> CommandMap;

struct GE_API CommandProcessor
{
public:
    Engine &_engine;
    CommandMap commands;
    std::vector<std::string> scriptDirs;

public:
    CommandProcessor(Engine &e) : _engine(e) {}
    size_t size() const;

    bool addScriptDirectory(std::string_view dir, bool check_exists = true);
    const std::vector<std::string> &scriptDirectories();

    Engine &engine() { return _engine; }

    CommandPtr command(std::string_view comname);

    bool define(CommandPtr comm, bool unique = false);

    bool define(std::string_view comname, CommandPtr comm, bool unique = false);

    bool exec(std::string_view comname, ArrayView<CommandArg> args);

    bool exec(CommandPtr &com,
              ArrayView<CommandArg> args,
              std::string_view comname = "");

    bool exec(ArrayView<CommandArg> args);

    bool execCommand(ArrayView<CommandArg> args);

    bool loadStream(sys::io::InStream &inp, std::string_view inp_name);

    bool loadScript(std::string_view name, bool quiet = false);

    bool evalCommand(std::string_view cmd);

    // CommandArg cast(const CommandArg &val, CommandType type);

    // static bool isAssignable(CommandParamType lval, CommandType rval);

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
