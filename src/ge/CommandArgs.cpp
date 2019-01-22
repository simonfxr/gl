#include "ge/CommandArgs.hpp"

#include "bl/range.hpp"
#include "bl/string.hpp"
#include "bl/string_view.hpp"
#include "ge/Command.hpp"
#include "ge/Commands.hpp"
#include "sys/io/Stream.hpp"

#include <algorithm>
#include <cstring>

namespace ge {

PP_DEF_ENUM_IMPL(GE_COMMAND_PARAM_TYPE_ENUM_DEF)

PP_DEF_ENUM_IMPL(GE_COMMAND_ARG_TYPE_ENUM_DEF)

namespace {

template<typename T>
void
destroy(T &x)
{
    x.~T();
}

template<typename T>
void
copy(T *dest, const T &x)
{
    new (dest) T(x);
}

template<typename T>
void
destructive_move(T *dest, T &&from)
{
    new (dest) T(bl::move(from));
    from.~T();
}

} // namespace

CommandValue::CommandValue(bl::shared_ptr<Command> com)
  : name(com, &com->name()), ref(bl::move(com))
{}

CommandArg::CommandArg(const CommandArg &rhs) : integer(), _type(rhs._type)
{
    switch (_type.value) {
    case CommandArgType::String:
        copy(&string, bl::move(rhs.string));
        break;
    case CommandArgType::Integer:
        integer = rhs.integer;
        break;
    case CommandArgType::Number:
        number = rhs.number;
        break;
    case CommandArgType::KeyCombo:
        copy(&keyBinding, bl::move(rhs.keyBinding));
        break;
    case CommandArgType::CommandRef:
        copy(&command, bl::move(rhs.command));
        break;
    case CommandArgType::VarRef:
        copy(&var, bl::move(rhs.var));
        break;
    case CommandArgType::Nil:
        break;
    }
}

CommandArg::CommandArg(CommandArg &&rhs) : CommandArg()
{
    *this = bl::move(rhs);
}

CommandArg::~CommandArg()
{
    reset();
}

CommandArg &
CommandArg::operator=(CommandArg &&rhs)
{
    if (this == &rhs)
        return *this;
    reset();
    _type = rhs._type;
    rhs._type = CommandArgType::Nil;
    switch (_type.value) {
    case CommandArgType::String:
        destructive_move(&string, bl::move(rhs.string));
        return *this;
    case CommandArgType::Integer:
        integer = rhs.integer;
        return *this;
    case CommandArgType::Number:
        number = rhs.number;
        return *this;
    case CommandArgType::KeyCombo:
        destructive_move(&keyBinding, bl::move(rhs.keyBinding));
        return *this;
    case CommandArgType::CommandRef:
        destructive_move(&command, bl::move(rhs.command));
        return *this;
    case CommandArgType::VarRef:
        destructive_move(&var, bl::move(rhs.var));
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
    switch (bl::exchange(_type.value, CommandArgType::Nil)) {
    case CommandArgType::String:
        destroy(string);
        return;
    case CommandArgType::Integer:
        return;
    case CommandArgType::Number:
        return;
    case CommandArgType::KeyCombo:
        destroy(keyBinding);
        return;
    case CommandArgType::CommandRef:
        destroy(command);
        return;
    case CommandArgType::VarRef:
        destroy(var);
        return;
    case CommandArgType::Nil:
        return;
    }
    UNREACHABLE;
}

int
compare(const CommandValue &a, const CommandValue &b)
{
    return compare(*a.name, *b.name);
}

int
compare(const CommandArg &a, const CommandArg &b)
{
    using bl::compare;
    if (a.type() != b.type())
        return compare(a.type(), b.type());
    switch (a.type().value) {
    case CommandArgType::String:
        return compare(a.string, b.string);
    case CommandArgType::Integer:
        return compare(a.integer, b.integer);
    case CommandArgType::Number:
        return compare(a.number, b.number);
    case CommandArgType::KeyCombo:
        return std::lexicographical_compare(a.keyBinding.begin(),
                                            a.keyBinding.end(),
                                            b.keyBinding.begin(),
                                            b.keyBinding.end());
    case CommandArgType::CommandRef:
        return compare(a.command, b.command);
    case CommandArgType::VarRef:
        return compare(a.var, b.var);
    case CommandArgType::Nil:
        return 0;
    }
    UNREACHABLE;
}

struct PrettyQuotation
{
    bl::vector<bl::string> statements;
    sys::io::ByteStream out;
    size_t len{};
};

struct CommandPrettyPrinter::Data
{
    sys::io::OutStream *out;
    sys::io::OutStream *current_out;

    size_t line_len{ 80 };
    size_t block_indent{ 4 };
    bool ignore_empty_statements{ true };

    bl::vector<bl::shared_ptr<PrettyQuotation>> quotations;

    Data() : out(&sys::io::stdout()), current_out(out) {}
};

DECLARE_PIMPL_DEL(CommandPrettyPrinter)

CommandPrettyPrinter::CommandPrettyPrinter() : self(new Data) {}

CommandPrettyPrinter::~CommandPrettyPrinter()
{
    flush();
}

void
CommandPrettyPrinter::out(sys::io::OutStream &_out)
{
    self->out = &_out;
    if (self->quotations.empty())
        self->current_out = self->out;
}

void
CommandPrettyPrinter::lineLength(size_t len)
{
    self->line_len = len;
}

void
CommandPrettyPrinter::blockIndent(size_t indent)
{
    self->block_indent = indent;
}

void
CommandPrettyPrinter::ignoreEmptyStatements(bool ignore)
{
    self->ignore_empty_statements = ignore;
}

void
CommandPrettyPrinter::print(const KeyBinding &bind)
{
    const char *sep = "[";

    for (const auto &k : bind) {
        *self->current_out << sep;
        sep = ", ";
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
            *self->current_out << pre;

        auto sym = to_string(k.code);
        if (!sym)
            *self->current_out << "<unknown key code: " << k.code.numeric()
                               << ">";
        else
            *self->current_out << sym;
    }

    *self->current_out << "]";
}

void
CommandPrettyPrinter::print(const CommandArg &arg, bool first)
{
    switch (arg.type().value) {
    case CommandArgType::String:
        *self->current_out << '"' << arg.string << '"';
        return;
    case CommandArgType::Integer:
        *self->current_out << arg.integer;
        return;
    case CommandArgType::Number:
        *self->current_out << arg.number;
        return;
    case CommandArgType::KeyCombo:
        print(arg.keyBinding);
        return;
    case CommandArgType::CommandRef: {
        if (!first)
            *self->current_out << '&';
        *self->current_out << *arg.command.name;
        if (!arg.command.ref)
            return;
        auto q = arg.command.ref->castToQuotation();
        if (q)
            print(*q->quotation);
        return;
    }
    case CommandArgType::VarRef:
        *self->current_out << '$' << arg.var;
        return;
    case CommandArgType::Nil:
        *self->current_out << "nil type not allowed: " << arg.type();
        return;
    }
    UNREACHABLE;
}

void
CommandPrettyPrinter::print(bl::array_view<const CommandArg> statement)
{

    if (self->ignore_empty_statements && statement.size() == 0)
        return;

    auto first = true;
    for (auto &stmt : statement) {
        *self->current_out << (first ? "" : " ");
        print(stmt, first);
    }
}

void
CommandPrettyPrinter::print(const Quotation &q)
{
    openQuotation();

    for (const auto &qi : q) {
        print(qi.view());
        auto &pq = self->quotations.back();
        pq->statements.push_back(pq->out.str());
        pq->out.truncate(0);
        pq->len += pq->statements.back().size();
    }

    closeQuotation();
}

void
CommandPrettyPrinter::openQuotation()
{
    self->quotations.push_back(bl::make_shared<PrettyQuotation>());
    self->current_out = &self->quotations.back()->out;
}

void
CommandPrettyPrinter::closeQuotation()
{
    // FIXME: output is ugly

    size_t depth = self->quotations.size();
    size_t indent = self->block_indent * depth;
    auto &q = self->quotations.back();
    size_t nln = q->statements.size();

    self->current_out =
      depth == 1 ? self->out : &self->quotations[depth - 2]->out;

    if (nln == 0) {
        printSpaces(indent);
        *self->current_out << "{}";
    } else {
        *self->current_out << "{";
        for (const auto &qi : q->statements) {
            printSpaces(indent);
            *self->current_out << qi << sys::io::endl;
        }
        *self->current_out << "}";
    }

    self->quotations.pop_back();
}

void
CommandPrettyPrinter::printSpaces(size_t n)
{
    for (auto i : bl::irange(n)) {
        UNUSED(i);
        *self->current_out << " ";
    }
}

void
CommandPrettyPrinter::flush()
{}

} // namespace ge
