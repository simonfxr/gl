#include "ge/Tokenizer.hpp"
#include "ge/Command.hpp"

#include <sstream>
#include <ctype.h>
#include <cmath>

#include "math/real.hpp"

namespace ge {

namespace {

enum State {
    EndToken,
    EndStatement,
    Fail
};

#define PARSE_ERROR(s, mesg) parse_err((s), _CURRENT_LOCATION, (mesg))

void parse_err(const ParseState& s, const err::Location& loc, const std::string& mesg) {
    if (s.in_state != sys::io::StreamBlocked) {
        std::ostringstream buf;
        buf << "parsing " << s.filename << "@" << s.line << ":" << s.col;
        buf << " parse-error: " << mesg;
        err::error(loc, ERROR_DEFAULT_STREAM, err::Error, buf.str());
    }
}

sys::io::StreamResult next(sys::io::InStream& in, char &c) {
    size s = 1;
    sys::io::StreamResult res = in.read(s, &c);
    ASSERT(res != sys::io::StreamOK || s == 1);
    return res;
}

bool getchAny(ParseState& s) {
    char lastC = s.rawC;

    s.in_state = next(*s.in, s.c);
    s.rawC = s.c;
    if (s.in_state != sys::io::StreamOK) {
        s.c = s.rawC = 0;
        return false;        
    }

    // std::cerr << "getch: '" << s.rawC << "'" << std::endl;

    if (s.c == '\n' || s.c == '\r') {
        if (lastC == '\r' && s.c == '\n') {
            return getchAny(s);
        } else {
            ++s.line;
            s.col = 0;
            s.c = s.rawC = '\n';
        }
    }

    ++s.col;
    return true;
}

bool getch(ParseState& s) {
    getchAny(s);
    bool ok = true;
    if (s.c == '#') {
        while (getchAny(s) && s.c != '\n')
            ;
        ok = s.c == '\n';
    }
    
    if (s.c == '\n')
        s.c = ';';

//    std::cerr << "getch: state = " << ok << ", eof = " << s.eof << ", c = " << s.c << std::endl;

    return ok;
}

void skipSpace(ParseState& s) {
    while (isspace(s.c))
        getch(s);
}

bool symFirst(char c) {
    if (isalpha(c))
        return true;
    switch (c) {
    case '_':
    case '?':
    case '*':
    case '%':
        return true;
    default:
        return false;
    }
}

bool symChar(char c) {
    return symFirst(c) || (c >= '0' && c <= '9');
}

std::string parseSym(ParseState& s) {
    std::ostringstream buf;

    if (!symFirst(s.c))
        return "";

    do {
        buf << s.c;
        getch(s);
    } while (symChar(s.c));

    return buf.str();
}

State parseKeycombo(ParseState& s, CommandArg& tok) {
    std::vector<Key> keys;

    bool needOne = true;

    getch(s);
    while (s.c != 0 && s.c != ']') {
        skipSpace(s);
        Key k;
        if (s.c == '!')
            k.state = Up;
        else if (s.c == '+')
            k.state = Pressed;
        else if (s.c == '-')
            k.state = Released;
        else
            k.state = Down;

        if (k.state != Down) {
            getch(s); skipSpace(s);
        }

        std::string sym = parseSym(s);
        
        if (sym.empty()) {
            PARSE_ERROR(s, "expected a key symbol");
            return Fail;
        }
        
        if (!parseKeyCode(sym, &k.code)) {
            PARSE_ERROR(s, "invalid key-symbol: " + sym);
            return Fail;
        }

        skipSpace(s);
        if (s.c == ',') {
            getch(s);
            needOne = true;
        } else {
            needOne = false;
        }
        keys.push_back(k);
    }

    if (s.c == 0) {
        if (needOne)
            PARSE_ERROR(s, "expected a key symbol");
        else
            PARSE_ERROR(s, "exptected ]");
        return Fail;
    }

    KeyBinding *bind = new KeyBinding(new Key[keys.size()], SIZE(keys.size()), KeyBinding::Owned);
    tok.keyBinding = bind;
    tok.type = KeyCombo;

    for (index i = 0; i < SIZE(keys.size()); ++i)
        (*bind)[i] = keys[size_t(i)];

    return EndToken;
}

State parseString(ParseState& s, CommandArg& tok) {
    char delim = s.c == '"'  ? '"' :
                 s.c == '\'' ? '\'' : ' ';
    
    std::ostringstream buf;
    if (delim != ' ') {
        getchAny(s);
        while (s.c != 0 && s.c != delim) {
            char suf;
            if (s.c == '\\') {
                getchAny(s);
                if (s.c == 0)
                    return Fail;
                char esc;
                switch (s.rawC) {
                case 'n': esc = '\n'; break;
                case 't': esc = '\t'; break;
                case 'r': esc = '\r'; break;
                case 'v': esc = '\v'; break;
                case '\\':
                case '"':
                case '\'': esc = s.rawC; break;
                default:
                    PARSE_ERROR(s, "invalid escape sequence in string literal");
                    return Fail;
                }

                suf = esc;
            } else {
                suf = s.rawC;
            }

            buf << suf;
            getchAny(s);
        }
    } else {
        while (s.c != 0 && s.c != ';' && !isspace(s.c)) {
            buf << s.c;
            getch(s);
        }
    }

    if (s.c == 0)
        return Fail;

    tok.type = String;
    tok.string = new std::string(buf.str());

    return EndToken;
}

State parseCommandRef(ParseState& s, CommandArg& arg) {
    if (s.c == 0)
        goto fail;
    { 
        std::string sym = parseSym(s);
        if (sym.empty())
            goto fail;
//         Ref<Command> comref = s.proc.command(sym);
//         if (!comref) {
// //            WARN(("unknown command name: " + sym));
//         }

        arg.type = CommandRef;
        arg.command.ref = new Ref<Command>();
        arg.command.name = new std::string(sym);
        arg.command.quotation = 0;

        return EndToken;
    }

fail:
    PARSE_ERROR(s, "expected a command name");
    return Fail;
}

State parseVarRef(ParseState& s, CommandArg& arg) {
    if (s.c == 0)
        goto fail;
    { 
        std::string sym = parseSym(s);
        if (sym.empty())
            goto fail;

        arg.type = VarRef;
        arg.var = new std::string(sym);
        return EndToken;
    }

fail:
    PARSE_ERROR(s, "expected a variable name");
    return Fail;
}

State statement(ParseState& s, std::vector<CommandArg>& toks, bool quot);

State parseQuot(ParseState& s, CommandArg& arg) {
    Quotation *q = new Quotation;

    int line = s.line;
    int col = s.col;
    
    for (;;) {
        getch(s);
        q->push_back(std::vector<CommandArg>());
        std::vector<CommandArg>& toks = (*q)[q->size() - 1];
        State st = statement(s, toks, true);

        if (st == Fail || toks.size() == 0)
            q->pop_back();

        if (st == Fail)
            return st;

        if (st != EndStatement)
            break;

        if (s.c == 0)
            break;
    }

    if (s.c != '}') {
        PARSE_ERROR(s, "expected }");
        return Fail;
    }

    arg.type = CommandRef;
    std::ostringstream buf;
    buf << "<quotation: " << s.filename << "@" << line << ":" << col << ">";
    arg.command.name = new std::string(buf.str());
    arg.command.ref = new Ref<Command>(new QuotationCommand(s.filename, line, col, "", q));
    arg.command.quotation = q;

    return EndToken;
}

int parsePow(ParseState& s) {
    int pow = 1;
    int sig = 1;

    if (s.c == 'E' || s.c == 'e') {
        getch(s);

        if (s.c == '-') {
            sig = -1;
            getch(s);
        } else if (s.c == '+') {
            getch(s);
        }

        pow = 0;
        while (s.c >= '0' && s.c <= '9') {
            pow = pow * 10 + s.c - '0';
            getch(s);
        }
    }

    return pow * sig;
}

State parseNum(ParseState& s, CommandArg& tok) {
    bool neg = false;
    while (s.c == '+' || s.c == '-') {
        if (s.c == '-')
            neg = !neg;
        getch(s); skipSpace(s);
    }

    bool oneDigit = false;
    int64 k = 0;
    while (s.c >= '0' && s.c <= '9') {
        k = (k * 10) + s.c - '0';
        getch(s);
        oneDigit = true;
    }

    double num = 0;
    bool isNum = false;

    if (oneDigit && (s.c == 'e' || s.c == 'E')) {
        int p = parsePow(s);
        num = ::pow(double(k), p);
        isNum = true;
    } else if (s.c == '.') {
        getch(s);
        int64 fract = 0;
        double div = 1;
        while (s.c >= '0' && s.c <= '9') {
            fract = fract * 10 + s.c - '0';
            div *= 10;
            oneDigit = true;
            getch(s);
        }

        int p = 0;
        if (s.c == 'e' || s.c == 'E')
            p = parsePow(s);

        num = (double(k) + double(fract) / div) * ::pow(10, p);
        isNum = true;
    }

    if (!oneDigit) {
        PARSE_ERROR(s, "expected a number");
        return Fail;
    }

    if (isNum) {
        tok.type = Number;
        tok.number = neg ? -num : num;
        // std::cerr << "parsed number: " << num << std::endl;
    } else {
        tok.type = Integer;
        tok.integer = neg ? -k : k;
        // std::cerr << "parsed int: " << k << std::endl;
    }

    return EndToken;
}

State token(ParseState& s, CommandArg& tok, bool first) {
    skipSpace(s);

    if (first) {
        switch (s.c) {
        case '{': return parseQuot(s, tok);
        default:
            return parseCommandRef(s, tok);
        }
    }
    
    switch (s.c) {
    case '[': return parseKeycombo(s, tok);
    case '\'':
    case '"': return parseString(s, tok);
    case '&': getch(s); return parseCommandRef(s, tok);
    case '$': getch(s); return parseVarRef(s, tok);
    case '{': return parseQuot(s, tok);
    default:
        if ((s.c >= '0' && s.c <= '9') || s.c == '+' || s.c == '-' || s.c == '.')
            return parseNum(s, tok);
        else if (symFirst(s.c))
            return parseString(s, tok);
        return Fail;
    }
}

State statement(ParseState& s, std::vector<CommandArg>& toks, bool quot) {
    State st;
    bool first = true;
    for (;;) {
        if (!first) getch(s);
        if (quot) {
            skipSpace(s);
            if (s.c == '}')
                return EndToken;
        }
        
        if (s.c == ';' || s.c == 0)
            break;
        CommandArg tok;
        st = token(s, tok, first);
        if (st != Fail)
            toks.push_back(tok);
        else
            return Fail;
        first = false;
        if (s.c == ';' || s.c == 0)
            break;
    }

    if (s.c == 0 && s.in_state != sys::io::StreamEOF)
        return Fail;

    return EndStatement;
}

} // namespace anon

bool skipStatement(ParseState& s) {
    while (getch(s) && s.c != ';')
        ;
    return s.c == ';';
}

bool tokenize(ParseState& s, std::vector<CommandArg>& args) {
    getch(s);
    if (s.in_state != sys::io::StreamOK)
        return false;
    return statement(s, args, false) != Fail;
}

} // namespace ge

