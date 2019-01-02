#include "ge/CommandArgs.hpp"

#include "data/range.hpp"
#include "ge/Command.hpp"
#include "sys/io/Stream.hpp"

#include <cstring>

namespace ge {

void
CommandArg::free()
{

    switch (type) {
    case String:
        delete string;
        break;
    case KeyCombo:
        delete keyBinding;
        break;
    case CommandRef:
        delete command.name;
        delete command.ref;
        // command.quotation gets deleted by command.ref
        break;
    case Integer:
    case Number:
    case VarRef:
    case Nil:
        break;
    }

    memset(this, 0, sizeof *this);
}

struct PrettyQuotation
{
    std::vector<std::string> statements;
    sys::io::ByteStream out;
    size_t len{};
};

struct CommandPrettyPrinter::State
{
    sys::io::OutStream *out;
    sys::io::OutStream *current_out;

    size_t line_len{ 80 };
    size_t block_indent{ 4 };
    bool ignore_empty_statements{ true };

    std::vector<std::shared_ptr<PrettyQuotation>> quotations;

    State() : out(&sys::io::stdout()), current_out(out) {}
};

CommandPrettyPrinter::CommandPrettyPrinter() : self(new State) {}

CommandPrettyPrinter::~CommandPrettyPrinter()
{
    flush();
    delete self;
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
    switch (arg.type) {
    case String:
        *self->current_out << '"' << *arg.string << '"';
        break;
    case Integer:
        *self->current_out << arg.integer;
        break;
    case Number:
        *self->current_out << arg.number;
        break;
    case KeyCombo:
        print(*arg.keyBinding);
        break;
    case CommandRef:
        if (!first)
            *self->current_out << '&';
        *self->current_out << *arg.command.name;
        if (arg.command.quotation != nullptr)
            print(*arg.command.quotation);
        break;
    case VarRef:
        *self->current_out << '$' << *arg.var;
        break;
    case Nil:
        *self->current_out << "nil type not allowed: " << arg.type;
        break;
    }
}

void
CommandPrettyPrinter::print(const Array<CommandArg> &statement)
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
CommandPrettyPrinter::print(const std::vector<CommandArg> &statement)
{
    auto arr = SHARE_ARRAY(const_cast<CommandArg *>(&statement.front()),
                           statement.size());
    print(arr);
}

void
CommandPrettyPrinter::print(const Quotation &q)
{
    openQuotation();

    for (const auto &qi : q) {
        print(qi);
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
