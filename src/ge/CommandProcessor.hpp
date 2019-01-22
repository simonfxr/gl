#ifndef GE_COMMAND_PROCESSOR_HPP
#define GE_COMMAND_PROCESSOR_HPP

#include "bl/array_view.hpp"
#include "bl/string.hpp"
#include "bl/vector.hpp"
#include "ge/Command.hpp"
#include "ge/CommandArgs.hpp"
#include "pp/pimpl.hpp"
#include "sys/io/Stream.hpp"

namespace ge {

struct Engine;

struct GE_API CommandProcessor
{
    CommandProcessor(Engine &e);
    ~CommandProcessor() noexcept;

    bool addScriptDirectory(bl::string dir, bool check_exists = true);
    bl::array_view<const bl::string> scriptDirectories() const;

    Engine &engine();

    CommandPtr command(bl::string_view comname) const;

    bool define(CommandPtr comm, bool unique = false);

    bool exec(bl::string_view comname, bl::array_view<CommandArg> args);

    bool exec(CommandPtr &com, bl::array_view<CommandArg> args);

    bool exec(bl::array_view<CommandArg> args);

    bool execCommand(bl::array_view<CommandArg> args);

    bool loadStream(sys::io::InStream &inp, bl::string_view inp_name);

    bool loadScript(bl::string_view name, bool quiet = false);

    bool evalCommand(bl::string_view cmd);

    bl::vector<CommandPtr> commands() const;

    static CommandParamType commandParamType(CommandArgType type);

private:
    DECLARE_PIMPL(GE_API, self);
};

} // namespace ge

#endif
