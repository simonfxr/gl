#include "ge/Command.hpp"

#include "ge/Engine.hpp"

#include "util/string.hpp"

namespace ge {

Command::~Command() = default;

CommandEvent::CommandEvent(ge::Engine &e, CommandProcessor &proc)
  : EngineEvent(e), processor(proc)
{}

Command::Command(bl::vector<CommandParamType> ps,
                 bl::string name_,
                 bl::string desc_)
  : params(ps), namestr(bl::move(name_)), descr(bl::move(desc_))
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
        ERR(
          string_concat("cannot execute command without arguments: ", name()));
    }
}

bl::string
Command::interactiveDescription() const
{
    sys::io::ByteStream desc;
    desc << "command " << name() << sys::io::endl << "  params:";

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

    desc << sys::io::endl
         << "  description: " << description() << sys::io::endl;

    return desc.str();
}

namespace {
bl::string
describeQuotation(bl::string_view &source,
                  int line,
                  int column,
                  bl::string_view &desc)
{
    sys::io::ByteStream str;
    str << "<quotation " << source << "@" << line << ":" << column;
    if (!desc.empty())
        str << " " << desc << ">";
    return str.str();
}
} // namespace

namespace {
bl::string
nameOfQuotation(bl::string_view filename, int line, int col)
{
    sys::io::ByteStream buf;
    buf << "<quotation: " << filename << "@" << line << ":" << col << ">";
    return buf.str();
}
} // namespace

QuotationCommand::QuotationCommand(bl::string_view source,
                                   int line,
                                   int column,
                                   bl::string_view desc,
                                   bl::unique_ptr<Quotation> quot)
  : Command({},
            nameOfQuotation(source, line, column),
            describeQuotation(source, line, column, desc))
  , quotation(bl::move(quot))
{}

QuotationCommand::~QuotationCommand() = default;

void
QuotationCommand::interactive(const Event<CommandEvent> &ev,
                              bl::array_view<const CommandArg> /*unused*/)
{
    for (auto &args : *quotation)
        ev.info.engine.commandProcessor().exec(args);
}

} // namespace ge
