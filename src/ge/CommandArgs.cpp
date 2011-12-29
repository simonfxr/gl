#include "ge/CommandArgs.hpp"
#include "ge/Command.hpp"

#include "sys/io/Stream.hpp"

#include <cstring>
#include <sstream>

namespace ge {

CommandArg::CommandArg() { memset(this, 0, sizeof *this); }

void CommandArg::free() {
    
    switch (type) {
    case String: delete string; break;
    case KeyCombo: delete keyBinding; break;
    case CommandRef:
        delete command.name;
        delete command.ref;
        // command.quotation gets deleted by command.ref
        break;
    case Integer:
    case Number:
    case VarRef:
    case Nil: break;
    }

    memset(this, 0, sizeof *this);
}

struct PrettyQuotation {
    std::vector<std::string> statements;
    std::ostringstream out;
    sys::io::StdOutStream out_stream;
    size len;

    PrettyQuotation() :
        out_stream(out),
        len(0)        
        {}
};

struct CommandPrettyPrinter::State {
    sys::io::OutStream *out;
    sys::io::OutStream *current_out;
    
    size line_len;
    size block_indent;
    bool ignore_empty_statements;

    std::vector<Ref<PrettyQuotation> > quotations;

    State() :
        out(&sys::io::stdout()),
        current_out(out),
        line_len(80),
        block_indent(4),
        ignore_empty_statements(true)
        {}
};

CommandPrettyPrinter::CommandPrettyPrinter() :
    self(new State)
{}

CommandPrettyPrinter::~CommandPrettyPrinter() {
    flush();
    delete self;
}

void CommandPrettyPrinter::out(sys::io::OutStream& _out) {
    self->out = &_out;
    if (self->quotations.empty())
        self->current_out = self->out;
}

void CommandPrettyPrinter::lineLength(size len) { self->line_len = len; }

void CommandPrettyPrinter::blockIndent(size indent) { self->block_indent = indent; }

void CommandPrettyPrinter::ignoreEmptyStatements(bool ignore) {
    self->ignore_empty_statements = ignore;
}

void CommandPrettyPrinter::print(const KeyBinding& bind) {
    const char *sep = "[";
    
    for (defs::index i = 0; i < bind.size(); ++i) {
        *self->current_out << sep;
        sep = ", ";
        const Key& k = bind[i];
        char pre = 0;
        switch (k.state) {
        case Up: pre = '!'; break;
        case Down: pre = 0; break;
        case Pressed: pre = '+'; break;
        case Released: pre = '-'; break;
        }
                
        if (pre != 0)
            *self->current_out << pre;
            
        const char *sym = prettyKeyCode(k.code);
        if (sym == 0)
            *self->current_out << "<unknown key code: " << k.code << ">";
        else
            *self->current_out << sym;
    }
    
    *self->current_out << "]";
}

void CommandPrettyPrinter::print(const CommandArg& arg, bool first) {
    switch (arg.type) {
    case String:
        *self->current_out << '"' << *arg.string << '"'; break;
    case Integer:
        *self->current_out << arg.integer; break;
    case Number:
        *self->current_out << arg.number; break;
    case KeyCombo: print(*arg.keyBinding); break;
    case CommandRef:
        if (!first)
            *self->current_out << '&';
        *self->current_out << *arg.command.name;
        if (arg.command.quotation != 0)
            print(*arg.command.quotation);
        break;
    case VarRef:
        *self->current_out << '$' << *arg.var; break;
    default:
        *self->current_out << "invalid type: " << arg.type;
    }
}

void CommandPrettyPrinter::print(const Array<CommandArg>& statement) {

    if (self->ignore_empty_statements && statement.size() == 0)
        return;

    const char *sep = "";
    for (defs::index i = 0; i < statement.size(); ++i) {
        *self->current_out << sep;
        print(statement[i], i == 0);
        sep = " ";
    }
}

void CommandPrettyPrinter::print(const std::vector<CommandArg>& statement) {
    Array<CommandArg> arr(const_cast<CommandArg *>(&statement.front()), SIZE(statement.size()));
    print(arr);
}

void CommandPrettyPrinter::print(const Quotation& q) {
    openQuotation();
    
    for (defs::index i = 0; i < SIZE(q.size()); ++i) {
        print(q[i]);
        Ref<PrettyQuotation>& pq = self->quotations.back();
        pq->statements.push_back(pq->out.str());
        pq->out.str("");
        pq->len += pq->statements.back().length();
    }
    
    closeQuotation();
}

void CommandPrettyPrinter::openQuotation() {
    self->quotations.push_back(makeRef(new PrettyQuotation));
    self->current_out = &self->quotations.back()->out_stream;
}

void CommandPrettyPrinter::closeQuotation() {

    // FIXME: output is ugly
    
    size depth = SIZE(self->quotations.size());
    size indent = self->block_indent * depth;
    Ref<PrettyQuotation>& q = self->quotations.back();
    size nln = q->statements.size();

    self->current_out = depth == 1 ? self->out : &self->quotations[depth - 2]->out_stream;
    
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

void CommandPrettyPrinter::printSpaces(size n) {
    for (size i = 0; i < n; ++i)
        *self->current_out << " ";
}
    
void CommandPrettyPrinter::flush() {
    
}

} // namespace ge
