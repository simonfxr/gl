#ifndef GE_COMMAND_PROCESSOR_HPP
#define GE_COMMAND_PROCESSOR_HPP

#include "ge/Command.hpp"
#include "ge/CommandArgs.hpp"
#include "pp/pimpl.hpp"
#include "sys/io/Stream.hpp"

#include <span>
#include <string>
#include <vector>

namespace ge {

struct Engine;

struct GE_API CommandProcessor
{
    CommandProcessor(Engine &e);
    ~CommandProcessor() noexcept;

    bool addScriptDirectory(std::string dir, bool check_exists = true);
    std::span<const std::string> scriptDirectories() const;

    Engine &engine();

    CommandPtr command(std::string_view comname) const;

    bool define(CommandPtr comm, bool unique = false);

    bool exec(std::string_view comname, std::span<CommandArg> args);

    bool exec(CommandPtr &com, std::span<CommandArg> args);

    bool exec(std::span<CommandArg> args);

    bool execCommand(std::span<CommandArg> args);

    bool loadStream(sys::io::InStream &inp, std::string_view inp_name);

    bool loadScript(std::string_view name, bool quiet = false);

    bool evalCommand(std::string_view cmd);

    std::vector<CommandPtr> commands() const;

    static CommandParamType commandParamType(CommandArgType type);

private:
    DECLARE_PIMPL(GE_API, self);
};

} // namespace ge

#endif
