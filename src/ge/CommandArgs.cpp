#include "ge/CommandArgs.hpp"
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
    size len{};
};

struct CommandPrettyPrinter::State
{
    sys::io::OutStream *out;
    sys::io::OutStream *current_out;

    size line_len{ 80 };
    size block_indent{ 4 };
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
CommandPrettyPrinter::lineLength(size len)
{
    self->line_len = len;
}

void
CommandPrettyPrinter::blockIndent(size indent)
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

    for (defs::index i = 0; i < bind.size(); ++i) {
        *self->current_out << sep;
        sep = ", ";
        const Key &k = bind[i];
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

    const char *sep = "";
    for (defs::index i = 0; i < statement.size(); ++i) {
        *self->current_out << sep;
        print(statement[i], i == 0);
        sep = " ";
    }
}

void
CommandPrettyPrinter::print(const std::vector<CommandArg> &statement)
{
    const Array<CommandArg> arr = SHARE_ARRAY(
      SIZE(statement.size()), const_cast<CommandArg *>(&statement.front()));
    print(arr);
}

void
CommandPrettyPrinter::print(const Quotation &q)
{
    openQuotation();

    for (defs::index i = 0; i < SIZE(q.size()); ++i) {
        print(q[i]);
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

    size depth = SIZE(self->quotations.size());
    size indent = self->block_indent * depth;
    auto &q = self->quotations.back();
    size nln = q->statements.size();

    self->current_out =
      depth == 1 ? self->out : &self->quotations[depth - 2]->out;

    if (nln == 0) {
        printSpaces(indent);
        *self->current_out << "{}";
    } else {
        *self->current_out << "{";
        for (defs::index i = 0; i < nln; ++i) {
            printSpaces(indent);
            *self->current_out << q->statements[i] << sys::io::endl;
        }
        *self->current_out << "}";
    }

    self->quotations.pop_back();
}

void
CommandPrettyPrinter::printSpaces(size n)
{
    for (size i = 0; i < n; ++i)
        *self->current_out << " ";
}

void
CommandPrettyPrinter::flush()
{}

} // namespace ge
