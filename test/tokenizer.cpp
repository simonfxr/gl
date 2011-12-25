#include "err/err.hpp"

#include "ge/Tokenizer.hpp"
#include "ge/Engine.hpp"
#include "ge/CommandProcessor.hpp"
#include "ge/KeyBinding.hpp"

#include <iostream>
#include <vector>
#include <sstream>

using namespace ge;
using namespace defs;

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

struct BufferedInput : public Input {
    bool from_buffer;
    std::vector<char> buffer;
    index buffer_pos;
    Ref<Input> in;
    BufferedInput(const Ref<Input>& _in) : from_buffer(false), in(_in) {}
    void Input::State next(char&);
    
    void readFromBuffer() {
        from_buffer = true;
        buffer_pos = 0;
    }

    void clearBuffer() {
        buffer_pos = 0;
        buffer.clear();
    }
};

Input::State BufferedInput::next(char& c) {
    if (from_buffer) {
        if (buffer_pos < SIZE(buffer.size())) {
            c = buffer[buffer_pos++];
            return Input::Ok;
        } else {
            from_buffer = false;
        }
    }

    Input::State s = in->next(c);
    if (s == Input::OK)
        buffer.push_back(c);
    
    return s;
}

enum TokenizeResult {
    TokenizeOk,
    TokenizeFailed,
    TokenizeIncomplete
};

TokenizeResult tryTokenize(ParseState& state, Ref<BufferedInput>& in, std::vector<ge::CommandArg>& args) {
    state.in = in.cast<Input>();
    in->readFromBuffer();
    index len = args.size();
    ParseState activeState = state;
    bool ok = tokenize(activeState, args);
    
    if (!ok && activeState.in_state == Input::Blocked) {
        args.erase(args.begin() + len, args.end());
        return TokenizeIncomplete;
    }

    in->clearBuffer();
    state = activeState;
    return ok ? TokenizeOk : TokenizeFailed;
}

int main(int argc, char *argv[]) {
    ge::Engine eng;
    ge::CommandProcessor proc(eng);
    ge::ParseState state(std::cin, proc, "<interactive>");
    Ref<std::istream> in_stream = 
    Ref<BufferedInput> in(new BufferedInput(makeRef(new IStreamInput(makeRef

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
