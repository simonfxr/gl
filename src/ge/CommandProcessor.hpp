#ifndef GE_COMMAND_PROCESSOR_HPP
#define GE_COMMAND_PROCESSOR_HPP

#include "ge/Command.hpp"
#include "ge/CommandArgs.hpp"
#include "sys/io/Stream.hpp"

#include <bl/new.hpp>

#include <bl/string.hpp>
#include <span>
#include <vector>

namespace ge {

struct Engine;

struct GE_API CommandProcessor
  : private bl::runtime_sized_base<CommandProcessor>
{
    static CommandProcessor &construct_at(void *p, Engine &e) noexcept;

    bool addScriptDirectory(bl::string dir, bool check_exists = true);
    std::span<const bl::string> scriptDirectories() const;

    Engine &engine();

    [[nodiscard]] CommandPtr command(bl::string_view comname) const;

    bool define(CommandPtr comm, bool unique = false);

    bool exec(bl::string_view comname, std::span<CommandArg> args);

    bool exec(const CommandPtr &com, std::span<CommandArg> args);

    bool exec(std::span<CommandArg> args);

    bool execCommand(std::span<CommandArg> args);

    bool loadStream(sys::io::InStream &inp, bl::string_view inp_name);

    bool loadScript(bl::string_view name, bool quiet = false);

    bool evalCommand(bl::string_view cmd);

    std::vector<CommandPtr> commands() const;

    static CommandParamType commandParamType(CommandArgType type);

    BL_DECLARE_RUNTIME_SIZED(CommandProcessor, Impl);
};

} // namespace ge

#endif
