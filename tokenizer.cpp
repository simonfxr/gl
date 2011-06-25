#include "error/error.hpp"

#include "ge/Tokenizer.hpp"
#include "ge/Engine.hpp"
#include "ge/CommandProcessor.hpp"
#include "ge/KeyBinding.hpp"

#include <iostream>
#include <vector>
#include <sstream>

using namespace ge;

static void printArgs(const std::vector<CommandArg>& args);

void printKeyCombo(const KeyBinding& bind) {
    const char *sep = "[";
    for (uint32 i = 0; i < bind.size(); ++i) {
        std::cout << sep;
        sep = ", ";
        const Key& k = bind[i];
        char pre;
        switch (k.state) {
        case Up: pre = '!'; break;
        case Down: pre = 0; break;
        case Pressed: pre = '+'; break;
        case Released: pre = '-'; break;
        }
                
        if (pre != 0)
            std::cout << pre;
            
        const char *sym = prettyKeyCode(k.code);
        if (sym == 0)
            std::cout << "<unknown key code: " << k.code;
        else
            std::cout << sym;
    }
    std::cout << "]";
}

void printQuot(const Quotation& q) {
    if (q.size() == 0) {
        std::cout << " { } ";
        return;
    }

    std::cout << "{ " << std::endl;
    for (uint32 i = 0; i < q.size(); ++i)
        printArgs(q[i]);
    std::cout << "}" << std::endl;
}

static void printArg(const ge::CommandArg& arg) {
    switch (arg.type) {
    case String:
        std::cout << '"' << *arg.string << '"'; break;
    case Integer:
        std::cout << arg.integer; break;
    case Number:
        std::cout << arg.number; break;
    case KeyCombo: printKeyCombo(*arg.keyBinding); break;
    case CommandRef:
        std::cout << '&' << *arg.command.name;
        if (arg.command.quotation != 0)
            printQuot(*arg.command.quotation);
        break;
    case VarRef:
        std::cout << '$' << *arg.var; break;
    default:
        std::cout << "invalid type: " << arg.type;
    }
}

static void printArgs(const std::vector<CommandArg>& args) {
    const char *sep = "";
    for (uint32 i = 0; i < args.size(); ++i) {
        std::cout << sep;
        printArg(args[i]);
        sep = " ";
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[]) {
    ge::Engine eng;
    ge::CommandProcessor proc(eng);
    ge::ParseState state(std::cin, proc, "<interactive>");

    while (std::cin.good()) {
        std::vector<ge::CommandArg> args;
        if (!ge::tokenize(state, args)) {
            ERR("tokenize failed");
            skipLine(state);
            continue;
        }

        printArgs(args);

        for (uint32 i = 0; i < args.size(); ++i)
            args[i].free();
    }
    
    return 0;
}
