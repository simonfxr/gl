#include "ge/Command.hpp"

#include "ge/Engine.hpp"
#include "util/string.hpp"

#include <utility>

using namespace bl::literals;

namespace ge {

Command::~Command() = default;

CommandEvent::CommandEvent(ge::Engine &e, CommandProcessor &proc)
  : EngineEvent(e), processor(proc)
{}

Command::Command(bl::vec<CommandParamType> ps,
                 bl::string name_,
                 bl::string desc_)
  : params(bl::move(ps)), namestr(bl::move(name_)), descr(bl::move(desc_))
{
    ASSERT(!namestr.empty());
}

void
Command::handle(const Event<CommandEvent> &ev)
{
    if (params.empty() ||
        (params.size() == 1 && params[0] == CommandParamType::List)) {
        interactive(ev, {});
    } else {
        ERR(scat("cannot execute command without arguments: "_sv, name()));
    }
}

bl::string
Command::interactiveDescription() const
{
    sys::io::ByteStream desc;
    desc << "command "_k << name() << "\n  params:"_k;

    auto delim = " "_k;
    for (const auto &param : parameters()) {
        desc << delim;
        delim = ", "_k;
        switch (param) {
        case CommandParamType::String:
            desc << "string"_k;
            break;
        case CommandParamType::Integer:
            desc << "int"_k;
            break;
        case CommandParamType::Number:
            desc << "num"_k;
            break;
        case CommandParamType::KeyCombo:
            desc << "key"_k;
            break;
        case CommandParamType::Command:
            desc << "command"_k;
            break;
        case CommandParamType::VarRef:
            desc << "var"_k;
            break;
        case CommandParamType::Any:
            desc << "?"_k;
            break;
        case CommandParamType::List:
            desc << "*"_k;
            break;
        }
    }

    if (parameters().empty())
        desc << " none"_k;

    desc << "\n"_k
         << "  description: "_k << description() << "\n"_k;

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
    str << "<quotation "_k << source << "@"_k << line << ":"_k << column;
    if (!desc.empty())
        str << " "_k << desc << ">"_k;
    return str.str();
}
} // namespace

namespace {
bl::string
nameOfQuotation(bl::string_view filename, int line, int col)
{
    sys::io::ByteStream buf;
    buf << "<quotation: "_k << filename << "@"_k << line << ":"_k << col
        << ">"_k;
    return buf.str();
}
} // namespace

QuotationCommand::QuotationCommand(bl::string_view source,
                                   int line,
                                   int column,
                                   bl::string_view desc,
                                   std::unique_ptr<Quotation> quot)
  : Command({},
            nameOfQuotation(source, line, column),
            describeQuotation(source, line, column, desc))
  , quotation(bl::move(quot))
{}

QuotationCommand::~QuotationCommand() = default;

void
QuotationCommand::interactive(const Event<CommandEvent> &ev,
                              bl::span<const CommandArg> /*unused*/)
{
    for (auto &args : *quotation)
        ev.info.engine.commandProcessor().exec(bl::span(args));
}

} // namespace ge
