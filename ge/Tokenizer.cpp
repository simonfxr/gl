#include "ge/Tokenizer.hpp"

#include <sstream>
#include <ctype.h>

namespace ge {

enum State {
    EndToken,
    EndStatement,
    Fail
};

bool seperator(char c) {
    switch (c) {
    case '\n':
    case '\r':
    case ';':
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case '#':
        return true;
    default:
        return false;
    };
}

char skipSpace(std::istream& in, State *state, bool first = false) {
    bool firstIter = true;
    char c;
    for (;;) {
        in >> c;
        if (!in.good()) {
            if (first && firstIter)
                *state = EndStatement;
            else
                *state = Fail;
            return 0;
        }

        firstIter = false;
        
        if (c == '\n' || c == '\r') {
            do {
                in >> c;
                if (!in.good()) break;
            } while (c == '\n' || c == '\r');
            *state = EndStatement;
            return 0;
        }

        if (c == '#') {
            *state = EndStatement;
            do {
                in >> c;
                if (!in.good()) break;
            } while (c != '\n' && c != '\r');
            return 0;
        }

        if (c == ';') {
            *state = EndStatement;
            return 0;
        }

        if (!isspace(c))
            return c;
    }
}

namespace {
State readToken(std::istream& in, CommandArg *arg) {
    arg->type = Integer;
    arg->integer = 0;

    State s = EndToken;

    bool num = false;
    bool neg = false;

    char c = skipSpace(in, &s, true);
    if (!c) goto ret;

    while (c == '+' || c == '-') {
        c = skipSpace(in, &s);
        if (!c) {
            s = Fail;
            goto ret;
        }
        if (c == '-') neg = !neg;
        num = true;
    }

    if (num || (c >= '0' && c <= '9') || c == '.') {
        double num;
        in >> num;
        if (!in.good()) {
            s = Fail;
            goto ret;
        }

        arg->type = Number;
        arg->number = neg ? - num : num;
        goto ret;
    }

    {
        char delim = 0;
        if (c == '"') delim = '"';
        else if (c == '\'') delim = '\'';
        
        std::ostringstream buf;
    
        for (;;) {
            in >> c;
            if (!in.good()) goto unmatched;
            if (c == delim || (delim == 0 && seperator(c)))
                break;
            buf << c;
        }

        arg->type = String;
        arg->string = new std::string(buf.str());
    }
    goto ret;

unmatched:
    s = Fail;
ret:
    return s;
}

} // namespace anon

bool tokenize(std::istream& in, std::vector<CommandArg>& args) {

    State s;
    std::string tok;
    do {
        args.push_back(CommandArg());
        s = readToken(in, &args[args.size() - 1]);
    } while (s == EndToken && in.good());
    args.pop_back();
    return s != Fail;
}

struct ParseState {
    char c;
    std::ifstream& in;
    int line;
    int col;
};



// bool tokenize(std::istream& in, std::vector<CommandArg>& args) {

// }

} // namespace ge

