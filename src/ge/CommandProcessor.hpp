#ifndef GE_COMMAND_PROCESSOR_HPP
#define GE_COMMAND_PROCESSOR_HPP

#include "ge/Command.hpp"
#include "ge/CommandArgs.hpp"

#include "data/Ref.hpp"

#include <map>
#include <vector>

namespace ge {

using namespace defs;

struct Engine;

struct CommandProcessor {
private:
    Engine& _engine;
    std::map<std::string, Ref<Command> > commands;
    std::vector<std::string> scriptDirs;
    
public:
    
    CommandProcessor(Engine& e) : _engine(e) {}
    size size() const;

    bool addScriptDirectory(const std::string& dir, bool check_exists = true);
    const std::vector<std::string>& scriptDirectories();

    Engine& engine() { return _engine; }
    
    Ref<Command> command(const std::string comname);

    bool define(const Ref<Command>& comm, bool unique = false);
    
    bool define(const std::string comname, const Ref<Command>& comm, bool unique = false);
    
    bool exec(const std::string& comname, Array<CommandArg>& args);
    
    bool exec(Ref<Command>& com, Array<CommandArg>& args, const std::string &com_name = "");

    bool exec(Array<CommandArg>& args);

    CommandArg cast(const CommandArg& val, CommandType type);

    static bool isAssignable(CommandParamType lval, CommandType rval);

    static CommandParamType commandParamType(CommandType type);

private:
    CommandProcessor(const CommandProcessor&);
    CommandProcessor& operator =(const CommandProcessor&);
};

const char *prettyCommandType(CommandType type);

const char *prettyCommandParamType(CommandParamType type);

} // namespace ge

#endif
