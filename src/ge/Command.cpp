#include "ge/Command.hpp"

#include "ge/Engine.hpp"

#include <utility>

namespace ge {

Command::~Command() = default;

CommandEvent::CommandEvent(ge::Engine &e, CommandProcessor &proc)
  : EngineEvent(e), processor(proc)
{}

Command::Command(std::vector<CommandParamType> ps,
                 std::string name_,
                 std::string desc_)
  : params(ps), namestr(std::move(name_)), descr(std::move(desc_))
{
    ASSERT(!namestr.empty());
}

void
Command::handle(const Event<CommandEvent> &ev)
{
    if (params.size() == 0 ||
        (params.size() == 1 && params[0] == CommandParamType::List)) {
        interactive(ev, {});
    } else {
        ERR("cannot execute command without arguments: " + name());
    }
}

std::string
Command::interactiveDescription() const
{
    sys::io::ByteStream desc;
    desc << "command " << name() << "\n  params:";

    const char *delim = " ";
    for (const auto &param : parameters()) {
        desc << delim;
        delim = ", ";
        switch (param.value) {
        case CommandParamType::String:
            desc << "string";
            break;
        case CommandParamType::Integer:
            desc << "int";
            break;
        case CommandParamType::Number:
            desc << "num";
            break;
        case CommandParamType::KeyCombo:
            desc << "key";
            break;
        case CommandParamType::Command:
            desc << "command";
            break;
        case CommandParamType::VarRef:
            desc << "var";
            break;
        case CommandParamType::Any:
            desc << "?";
            break;
        case CommandParamType::List:
            desc << "*";
            break;
        }
    }

    if (parameters().size() == 0)
        desc << " none";

    desc << "\n"
         << "  description: " << description() << "\n";

    return desc.str();
}

namespace {
std::string
describeQuotation(std::string_view &source,
                  int line,
                  int column,
                  std::string_view &desc)
{
    sys::io::ByteStream str;
    str << "<quotation " << source << "@" << line << ":" << column;
    if (!desc.empty())
        str << " " << desc << ">";
    return str.str();
}
} // namespace

namespace {
std::string
nameOfQuotation(std::string_view filename, int line, int col)
{
    sys::io::ByteStream buf;
    buf << "<quotation: " << filename << "@" << line << ":" << col << ">";
    return buf.str();
}
} // namespace

QuotationCommand::QuotationCommand(std::string_view source,
                                   int line,
                                   int column,
                                   std::string_view desc,
                                   std::unique_ptr<Quotation> quot)
  : Command({},
            nameOfQuotation(source, line, column),
            describeQuotation(source, line, column, desc))
  , quotation(std::move(quot))
{}

QuotationCommand::~QuotationCommand() = default;

void
QuotationCommand::interactive(const Event<CommandEvent> &ev,
                              ArrayView<const CommandArg> /*unused*/)
{
    for (auto &args : *quotation)
        ev.info.engine.commandProcessor().exec(view_array(args));
}

} // namespace ge
