#ifndef GE_COMMAND_PROCESSOR_HPP
#define GE_COMMAND_PROCESSOR_HPP

#include "ge/Command.hpp"
#include "ge/CommandArgs.hpp"

#include "data/Ref.hpp"

#include <map>

namespace ge {

struct Engine;

struct CommandProcessor {
private:
    Engine& _engine;
    std::map<std::string, Ref<Command> > commands;
    
public:
    
    CommandProcessor(Engine& e) : _engine(e) {}
    uint32 size() const;

    Engine& engine() { return _engine; }
    
    Ref<Command> command(const std::string comname);
    
    bool define(const std::string comname, const Ref<Command>& comm, bool unique = false);
    
    bool exec(const std::string& comname, Array<CommandArg>& args);
    
    bool exec(Ref<Command>& com, Array<CommandArg>& args, const std::string &com_name = "");

private:
    CommandProcessor(const CommandProcessor&);
    CommandProcessor& operator =(const CommandProcessor&);
};

const char *prettyCommandType(CommandType type);

const char *prettyCommandParamType(CommandParamType type);

} // namespace ge

#endif
