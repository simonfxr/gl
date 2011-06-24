#include "ge/CommandArgs.hpp"
#include "ge/Command.hpp"

#include <cstring>

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
    }

    memset(this, 0, sizeof *this);
}

void prettyKeyCombo(std::ostream& out, const KeyBinding& bind) {
    const char *sep = "[";
    for (uint32 i = 0; i < bind.size(); ++i) {
        out << sep;
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
            out << pre;
            
        const char *sym = prettyKeyCode(k.code);
        if (sym == 0)
            out << "<unknown key code: " << k.code << ">";
        else
            out << sym;
    }
    out << "]";
}

void prettyQuot(std::ostream& out, const Quotation& q) {
    if (q.size() == 0) {
        out << " { } ";
        return;
    }

    out << "{ " << std::endl;
    for (uint32 i = 0; i < q.size(); ++i)
        prettyCommandArgs(out, Array<CommandArg>(const_cast<CommandArg *>(&q[i][0]), q[i].size()));
    out << "}" << std::endl;
}

void prettyCommandArg(std::ostream& out, const ge::CommandArg& arg) {
    switch (arg.type) {
    case String:
        out << '"' << *arg.string << '"'; break;
    case Integer:
        out << arg.integer; break;
    case Number:
        out << arg.number; break;
    case KeyCombo: prettyKeyCombo(out, *arg.keyBinding); break;
    case CommandRef:
        out << '&' << *arg.command.name;
        if (arg.command.quotation != 0)
            prettyQuot(out, *arg.command.quotation);
        break;
    case VarRef:
        out << '$' << *arg.var; break;
    default:
        out << "invalid type: " << arg.type;
    }
}

void prettyCommandArgs(std::ostream& out, const Array<CommandArg>& args) {
    const char *sep = "";
    for (uint32 i = 0; i < args.size(); ++i) {
        out << sep;
        prettyCommandArg(out, args[i]);
        sep = " ";
    }
    out << std::endl;
}

} // namespace ge
