#ifndef GE_COMMAND_REGISTRY_HPP
#define GE_COMMAND_REGISTRY_HPP

#include "ge/Command.hpp"
#include "ge/CommandArgs.hpp"

#include "data/Ref.hpp"

#include <map>

namespace ge {

struct Engine;

struct CommandRegistry {
private:
    Engine& _engine;
    std::map<std::string, Ref<Command> > commands;
    
public:
    
    CommandRegistry(Engine& e) : _engine(e) {}
    uint32 size() const;

    Engine& engine() { return _engine; }
    
    Ref<Command> command(const std::string comname);
    
    bool define(const std::string comname, const Ref<Command>& comm, bool unique = false);
    
    bool exec(const std::string& comname, Array<CommandArg>& args);
    
    bool exec(Ref<Command>& com, Array<CommandArg>& args, const std::string &com_name = "");

private:
    CommandRegistry(const CommandRegistry&);
    CommandRegistry& operator =(const CommandRegistry&);
};

const char *prettyCommandType(CommandType type);

const char *prettyCommandParamType(CommandParamType type);

} // namespace ge

#endif
