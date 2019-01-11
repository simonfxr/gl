#include "ge/CommandArgs.hpp"

#include "data/range.hpp"
#include "ge/Command.hpp"
#include "ge/Commands.hpp"
#include "sys/io/Stream.hpp"

#include <algorithm>
#include <cstring>

namespace ge {

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
    new (dest) T(std::move(from));
    from.~T();
}

} // namespace

CommandValue::CommandValue(std::shared_ptr<Command> com)
  : name(com, &com->name()), ref(std::move(com))
{}

CommandArg::CommandArg(const CommandArg &rhs) : integer(), _type(rhs._type)
{
    switch (_type) {
    case String:
        copy(&string, std::move(rhs.string));
        break;
    case Integer:
        integer = rhs.integer;
        break;
    case Number:
        number = rhs.number;
        break;
    case KeyCombo:
        copy(&keyBinding, std::move(rhs.keyBinding));
        break;
    case CommandRef:
        copy(&command, std::move(rhs.command));
        break;
    case VarRef:
        copy(&var, std::move(rhs.var));
        break;
    case Nil:
        break;
    }
}

CommandArg::CommandArg(CommandArg &&rhs) : CommandArg()
{
    *this = std::move(rhs);
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
    rhs._type = Nil;
    switch (_type) {
    case String:
        destructive_move(&string, std::move(rhs.string));
        return *this;
    case Integer:
        integer = rhs.integer;
        return *this;
    case Number:
        number = rhs.number;
        return *this;
    case KeyCombo:
        destructive_move(&keyBinding, std::move(rhs.keyBinding));
        return *this;
    case CommandRef:
        destructive_move(&command, std::move(rhs.command));
        return *this;
    case VarRef:
        destructive_move(&var, std::move(rhs.var));
        return *this;
    case Nil:
        return *this;
    }
    CASE_UNREACHABLE;
}

CommandArg &
CommandArg::operator=(const CommandArg &rhs)
{
    return *this = CommandArg(rhs);
}

void
CommandArg::reset()
{
    switch (std::exchange(_type, Nil)) {
    case String:
        destroy(string);
        return;
    case Integer:
        return;
    case Number:
        return;
    case KeyCombo:
        destroy(keyBinding);
        return;
    case CommandRef:
        destroy(command);
        return;
    case VarRef:
        destroy(var);
        return;
    case Nil:
        return;
    }
    CASE_UNREACHABLE;
}

int
compare(const CommandValue &a, const CommandValue &b)
{
    return a.name->compare(*b.name);
}

int
compare(const CommandArg &a, const CommandArg &b)
{
    using ::compare;
    if (a.type() != b.type())
        return compare(a.type(), b.type());
    switch (a.type()) {
    case String:
        return a.string.compare(b.string);
    case Integer:
        return compare(a.integer, b.integer);
    case Number:
        return compare(a.number, b.number);
    case KeyCombo:
        return std::lexicographical_compare(a.keyBinding.begin(),
                                            a.keyBinding.end(),
                                            b.keyBinding.begin(),
                                            b.keyBinding.end());
    case CommandRef:
        return compare(a.command, b.command);
    case VarRef:
        return a.var.compare(b.var);
    case Nil:
        return 0;
    }
    CASE_UNREACHABLE;
}

struct PrettyQuotation
{
    std::vector<std::string> statements;
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

    std::vector<std::shared_ptr<PrettyQuotation>> quotations;

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
        case keystate::Up:
            pre = '!';
            break;
        case keystate::Down:
            pre = 0;
            break;
        case keystate::Pressed:
            pre = '+';
            break;
        case keystate::Released:
            pre = '-';
            break;
        }

        if (pre != 0)
            *self->current_out << pre;

        const char *sym = prettyKeyCode(k.code);
        if (sym == nullptr)
            *self->current_out << "<unknown key code: " << k.code << ">";
        else
            *self->current_out << sym;
    }

    *self->current_out << "]";
}

void
CommandPrettyPrinter::print(const CommandArg &arg, bool first)
{
    switch (arg.type()) {
    case String:
        *self->current_out << '"' << arg.string << '"';
        return;
    case Integer:
        *self->current_out << arg.integer;
        return;
    case Number:
        *self->current_out << arg.number;
        return;
    case KeyCombo:
        print(arg.keyBinding);
        return;
    case CommandRef: {
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
    case VarRef:
        *self->current_out << '$' << arg.var;
        return;
    case Nil:
        *self->current_out << "nil type not allowed: " << arg.type();
        return;
    }
    CASE_UNREACHABLE;
}

void
CommandPrettyPrinter::print(ArrayView<const CommandArg> statement)
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
        print(view_array(qi));
        auto &pq = self->quotations.back();
        pq->statements.push_back(pq->out.str());
        pq->out.truncate(0);
        pq->len += pq->statements.back().length();
    }

    closeQuotation();
}

void
CommandPrettyPrinter::openQuotation()
{
    self->quotations.push_back(std::make_shared<PrettyQuotation>());
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
    for (auto i : irange(n)) {
        UNUSED(i);
        *self->current_out << " ";
    }
}

void
CommandPrettyPrinter::flush()
{}

} // namespace ge
