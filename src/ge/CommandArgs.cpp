#include "ge/CommandArgs.hpp"

#include "ge/Command.hpp"
#include "ge/Commands.hpp"
#include "sys/io/Stream.hpp"
#include "util/range.hpp"

#include <algorithm>
#include <cstring>

namespace ge {

using namespace bl::literals;

namespace {

template<typename T>
void
copy(T *dest, const T &x)
{
    new (dest) T(x);
}

} // namespace

CommandValue::CommandValue(std::shared_ptr<Command> com)
  : name(com, &com->name()), ref(bl::move(com))
{}

CommandArg::CommandArg(const CommandArg &rhs) : integer(), _type(rhs._type)
{
    switch (_type) {
    case CommandArgType::String:
        copy(&string, rhs.string);
        break;
    case CommandArgType::Integer:
        integer = rhs.integer;
        break;
    case CommandArgType::Number:
        number = rhs.number;
        break;
    case CommandArgType::KeyCombo:
        copy(&keyBinding, rhs.keyBinding);
        break;
    case CommandArgType::CommandRef:
        copy(&command, rhs.command);
        break;
    case CommandArgType::VarRef:
        copy(&var, rhs.var);
        break;
    case CommandArgType::Nil:
        break;
    }
}

CommandArg::CommandArg(CommandArg &&rhs) noexcept : CommandArg()
{
    *this = bl::move(rhs);
}

CommandArg::~CommandArg()
{
    reset();
}

CommandArg &
CommandArg::operator=(CommandArg &&rhs) noexcept
{
    if (this == &rhs)
        return *this;
    reset();
    _type = rhs._type;
    rhs._type = CommandArgType::Nil;
    switch (_type) {
    case CommandArgType::String:
        bl::relocate_at(&rhs.string, &string);
        return *this;
    case CommandArgType::Integer:
        integer = rhs.integer;
        return *this;
    case CommandArgType::Number:
        number = rhs.number;
        return *this;
    case CommandArgType::KeyCombo:
        bl::relocate_at(&rhs.keyBinding, &keyBinding);
        return *this;
    case CommandArgType::CommandRef:
        bl::relocate_at(&rhs.command, &command);
        return *this;
    case CommandArgType::VarRef:
        bl::relocate_at(&rhs.var, &var);
        return *this;
    case CommandArgType::Nil:
        return *this;
    }
    UNREACHABLE;
}

CommandArg &
CommandArg::operator=(const CommandArg &rhs)
{
    return *this = CommandArg(rhs);
}

void
CommandArg::reset()
{
    switch (bl::exchange(_type, CommandArgType::Nil)) {
    case CommandArgType::String:
        bl::destroy_at(&string);
        return;
    case CommandArgType::Integer:
    case CommandArgType::Number:
        return;
    case CommandArgType::KeyCombo:
        bl::destroy_at(&keyBinding);
        return;
    case CommandArgType::CommandRef:
        bl::destroy_at(&command);
        return;
    case CommandArgType::VarRef:
        bl::destroy_at(&var);
        return;
    case CommandArgType::Nil:
        return;
    }
    UNREACHABLE;
}

std::weak_ordering
operator<=>(const CommandValue &a, const CommandValue &b) noexcept
{
    return *a.name <=> *b.name;
}

std::weak_ordering
operator<=>(const CommandArg &a, const CommandArg &b) noexcept
{
    if (a.type() != b.type())
        return a.type() <=> b.type();
    switch (a.type()) {
    case CommandArgType::String:
        return a.string <=> b.string;
    case CommandArgType::Integer:
        return a.integer <=> b.integer;
    case CommandArgType::Number:
        return (a.number < b.number ? -1 : a.number > b.number ? 1 : 0) <=> 0;
    case CommandArgType::KeyCombo:
        return a.keyBinding <=> b.keyBinding;
    case CommandArgType::CommandRef:
        return a.command <=> b.command;
    case CommandArgType::VarRef:
        return a.var <=> b.var;
    case CommandArgType::Nil:
        return 0 <=> 0;
    }
    UNREACHABLE;
}

struct PrettyQuotation
{
    std::vector<bl::string> statements;
    sys::io::ByteStream out;
    size_t len{};
};

struct CommandPrettyPrinter::Impl : public CommandPrettyPrinter
{
    sys::io::OutStream *out;
    sys::io::OutStream *current_out;

    size_t line_len{ 80 };
    size_t block_indent{ 4 };
    bool ignore_empty_statements{ true };

    std::vector<std::shared_ptr<PrettyQuotation>> quotations;

    Impl() : out(&sys::io::stdout()), current_out(out) {}
    ~Impl() { flush(); }
};

BL_DEFINE_RUNTIME_SIZED(CommandPrettyPrinter, CommandPrettyPrinter::Impl);

CommandPrettyPrinter &
CommandPrettyPrinter::construct_at(void *p) noexcept
{
    return construct_impl(p);
}

void
CommandPrettyPrinter::out(sys::io::OutStream &_out)
{
    m().out = &_out;
    if (m().quotations.empty())
        m().current_out = m().out;
}

void
CommandPrettyPrinter::lineLength(size_t len)
{
    m().line_len = len;
}

void
CommandPrettyPrinter::blockIndent(size_t indent)
{
    m().block_indent = indent;
}

void
CommandPrettyPrinter::ignoreEmptyStatements(bool ignore)
{
    m().ignore_empty_statements = ignore;
}

void
CommandPrettyPrinter::print(const KeyBinding &bind)
{
    auto sep = "["_k;

    for (const auto &k : bind) {
        *m().current_out << sep;
        sep = ", "_k;
        char pre = 0;
        switch (k.state) {
        case KeyState::Up:
            pre = '!';
            break;
        case KeyState::Down:
            pre = 0;
            break;
        case KeyState::Pressed:
            pre = '+';
            break;
        case KeyState::Released:
            pre = '-';
            break;
        }

        if (pre != 0)
            *m().current_out << pre;

        auto sym = to_string(k.code);
        *m().current_out << sym;
    }

    *m().current_out << "]"_k;
}

void
CommandPrettyPrinter::print(const CommandArg &arg, bool first)
{
    switch (arg.type()) {
    case CommandArgType::String:
        *m().current_out << '"' << arg.string << '"';
        return;
    case CommandArgType::Integer:
        *m().current_out << arg.integer;
        return;
    case CommandArgType::Number:
        *m().current_out << arg.number;
        return;
    case CommandArgType::KeyCombo:
        print(arg.keyBinding);
        return;
    case CommandArgType::CommandRef: {
        if (!first)
            *m().current_out << '&';
        *m().current_out << *arg.command.name;
        if (!arg.command.ref)
            return;
        auto q = arg.command.ref->castToQuotation();
        if (q)
            print(*q->quotation);
        return;
    }
    case CommandArgType::VarRef:
        *m().current_out << '$' << arg.var;
        return;
    case CommandArgType::Nil:
        *m().current_out << "nil type not allowed: "_k << arg.type();
        return;
    }
    UNREACHABLE;
}

void
CommandPrettyPrinter::print(bl::span<const CommandArg> statement)
{

    if (m().ignore_empty_statements && statement.empty())
        return;

    auto first = true;
    for (auto &stmt : statement) {
        *m().current_out << (first ? ""_k : " "_k);
        print(stmt, first);
    }
}

void
CommandPrettyPrinter::print(const Quotation &q)
{
    openQuotation();

    for (const auto &qi : q) {
        print(bl::span(qi));
        auto &pq = m().quotations.back();
        pq->statements.push_back(pq->out.str());
        pq->out.truncate(0);
        pq->len += pq->statements.back().size();
    }

    closeQuotation();
}

void
CommandPrettyPrinter::openQuotation()
{
    m().quotations.push_back(std::make_shared<PrettyQuotation>());
    m().current_out = &m().quotations.back()->out;
}

void
CommandPrettyPrinter::closeQuotation()
{
    // FIXME: output is ugly

    auto depth = m().quotations.size();
    auto indent = m().block_indent * depth;
    auto &q = m().quotations.back();
    auto nln = q->statements.size();

    m().current_out = depth == 1 ? m().out : &m().quotations[depth - 2]->out;

    if (nln == 0) {
        printSpaces(indent);
        *m().current_out << "{}"_k;
    } else {
        *m().current_out << "{"_k;
        for (const auto &qi : q->statements) {
            printSpaces(indent);
            *m().current_out << qi << "\n"_k;
        }
        *m().current_out << "}"_k;
    }

    m().quotations.pop_back();
}

void
CommandPrettyPrinter::printSpaces(size_t n)
{
    for (auto i : irange(n)) {
        UNUSED(i);
        *m().current_out << " "_k;
    }
}

void
CommandPrettyPrinter::flush()
{}

} // namespace ge
