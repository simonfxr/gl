#include "ge/Tokenizer.hpp"
#include "ge/Command.hpp"

#include <cctype>
#include <cmath>

#include "math/real.hpp"

namespace ge {

namespace {

enum State
{
    EndToken,
    EndStatement,
    Fail
};

#define PARSE_ERROR(s, mesg) parse_err((s), ERROR_LOCATION, (mesg))

void
parse_err(const ParserState &s,
          const err::Location *loc,
          const std::string &mesg)
{
    if (s.in_state != sys::io::StreamResult::Blocked) {
        sys::io::ByteStream buf;
        buf << "parsing " << s.filename << "@" << s.line << ":" << s.col;
        buf << " parse-error: " << mesg;
        err::error(loc, err::LogLevel::Error, buf);
    }
}

sys::io::StreamResult
next(sys::io::InStream &in, char &c)
{
    auto [s, res] = in.read(std::span{ &c, size_t{ 1 } });
    ASSERT(res != sys::io::StreamResult::OK || s == 1);
    return res;
}

bool
getchAny(ParserState &s)
{
    char lastC = s.rawC;

    s.in_state = next(*s.in, s.c);
    s.rawC = s.c;
    if (s.in_state != sys::io::StreamResult::OK) {
        s.c = s.rawC = 0;
        return false;
    }

    //    sys::io::stdout() << "getch: '" << s.rawC << "'\n";

    if (s.c == '\n' || s.c == '\r') {
        if (lastC == '\r' && s.c == '\n') {
            return getchAny(s);
        }
        ++s.line;
        s.col = 0;
        s.c = s.rawC = '\n';
    }

    ++s.col;
    return true;
}

bool
getch(ParserState &s)
{
    getchAny(s);
    bool ok = true;
    if (s.c == '#') {
        while (getchAny(s) && s.c != '\n')
            ;
        ok = s.c == '\n';
    }

    if (s.c == '\n')
        s.c = ';';

    //    std::cerr << "getch: state = " << ok << ", eof = " << s.eof << ", c =
    //    " << s.c << std::endl;

    return ok;
}

void
skipSpace(ParserState &s)
{

    while (isspace(s.c))
        getch(s);
}

bool
symFirst(char c)
{
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

bool
symChar(char c)
{
    return symFirst(c) || (c >= '0' && c <= '9') || (c == '.');
}

std::string
parseSym(ParserState &s)
{
    sys::io::ByteStream buf;

    if (!symFirst(s.c))
        return "";

    do {
        buf << s.c;
        getch(s);
    } while (symChar(s.c));

    return buf.str();
}

State
parseKeycombo(ParserState &s, CommandArg &tok)
{
    std::vector<Key> keys;

    bool needOne = true;

    getch(s);
    while (s.c != 0 && s.c != ']') {
        skipSpace(s);
        Key k{};
        if (s.c == '!')
            k.state = KeyState::Up;
        else if (s.c == '+')
            k.state = KeyState::Pressed;
        else if (s.c == '-')
            k.state = KeyState::Released;
        else
            k.state = KeyState::Down;

        if (k.state != KeyState::Down) {
            getch(s);
            skipSpace(s);
        }

        std::string sym = parseSym(s);

        if (sym.empty()) {
            PARSE_ERROR(s, "expected a key symbol");
            return Fail;
        }

        if (auto opt_kcode = parseKeyCode(sym)) {
            k.code = *opt_kcode;
        } else {
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

    tok = CommandArg(std::move(keys));
    return EndToken;
}

State
parseString(ParserState &s, CommandArg &tok)
{
    char delim = s.c == '"' ? '"' : s.c == '\'' ? '\'' : ' ';

    sys::io::ByteStream buf;
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
                case 'n':
                    esc = '\n';
                    break;
                case 't':
                    esc = '\t';
                    break;
                case 'r':
                    esc = '\r';
                    break;
                case 'v':
                    esc = '\v';
                    break;
                case '\\':
                case '"':
                case '\'':
                    esc = s.rawC;
                    break;
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

    tok = CommandArg(buf.str());
    return EndToken;
}

State
parseCommandRef(ParserState &s, CommandArg &arg)
{
    if (s.c == 0)
        goto fail;
    {
        std::string sym = parseSym(s);
        if (sym.empty())
            goto fail;
        //         CommandPtr comref = s.proc.command(sym);
        //         if (!comref) {
        // //            WARN(("unknown command name: " + sym));
        //         }

        arg = CommandArg::namedCommandRef(std::move(sym));
        return EndToken;
    }

fail:
    PARSE_ERROR(s, "expected a command name");
    return Fail;
}

State
parseVarRef(ParserState &s, CommandArg &arg)
{
    if (s.c == 0)
        goto fail;
    {
        std::string sym = parseSym(s);
        if (sym.empty())
            goto fail;

        arg = CommandArg::varRef(std::move(sym));
        return EndToken;
    }

fail:
    PARSE_ERROR(s, "expected a variable name");
    return Fail;
}

State
statement(ParserState &s, std::vector<CommandArg> &toks, bool quot);

State
parseQuot(ParserState &s, CommandArg &arg)
{
    auto q = std::make_unique<Quotation>();

    int line = s.line;
    int col = s.col;

    for (;;) {
        getch(s);
        q->push_back(std::vector<CommandArg>());
        std::vector<CommandArg> &toks = (*q)[q->size() - 1];
        State st = statement(s, toks, true);

        if (st == Fail || toks.empty())
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

    arg = CommandArg(std::make_shared<QuotationCommand>(
      s.filename, line, col, "", std::move(q)));

    return EndToken;
}

int
parsePow(ParserState &s)
{
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

State
parseNum(ParserState &s, CommandArg &tok)
{
    bool neg = false;
    while (s.c == '+' || s.c == '-') {
        if (s.c == '-')
            neg = !neg;
        getch(s);
        skipSpace(s);
    }

    bool oneDigit = false;
    int64_t k = 0;
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
        int64_t fract = 0;
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

        num = (double(k) + double(fract) / div) * ::pow(10.0, p);
        isNum = true;
    }

    if (!oneDigit) {
        PARSE_ERROR(s, "expected a number");
        return Fail;
    }

    if (isNum) {
        tok = CommandArg(neg ? -num : num);
        // std::cerr << "parsed number: " << num << std::endl;
    } else {
        tok = CommandArg(neg ? -k : k);
        // std::cerr << "parsed int: " << k << std::endl;
    }

    return EndToken;
}

State
token(ParserState &s, CommandArg &tok, bool first)
{
    skipSpace(s);

    if (first) {
        switch (s.c) {
        case '{':
            return parseQuot(s, tok);
        default:
            return parseCommandRef(s, tok);
        }
    }

    switch (s.c) {
    case '[':
        return parseKeycombo(s, tok);
    case '\'':
    case '"':
        return parseString(s, tok);
    case '&':
        getch(s);
        return parseCommandRef(s, tok);
    case '$':
        getch(s);
        return parseVarRef(s, tok);
    case '{':
        return parseQuot(s, tok);
    default:
        if ((s.c >= '0' && s.c <= '9') || s.c == '+' || s.c == '-' ||
            s.c == '.')
            return parseNum(s, tok);
        else if (symFirst(s.c))
            return parseString(s, tok);
        return Fail;
    }
}

State
statement(ParserState &s, std::vector<CommandArg> &toks, bool quot)
{
    State st;
    bool first = true;
    for (;;) {
        if (!first) {
            getch(s);
        }
        if (quot) {
            skipSpace(s);
            if (s.c == '}')
                return EndToken;
        }

        if (s.c == ';' || s.c == 0)
            break;
        CommandArg tok{};
        st = token(s, tok, first);
        if (st != Fail)
            toks.push_back(tok);
        else
            return Fail;
        first = false;
        if (s.c == ';' || s.c == 0)
            break;
    }

    if (s.c == 0 && s.in_state != sys::io::StreamResult::EOF)
        return Fail;

    return EndStatement;
}

} // namespace

bool
skipStatement(ParserState &s)
{
    while (getch(s) && s.c != ';')
        ;
    return s.c == ';';
}

bool
tokenize(ParserState &s, std::vector<CommandArg> &args)
{
    getch(s);
    if (s.in_state != sys::io::StreamResult::OK)
        return false;
    return statement(s, args, false) != Fail;
}

} // namespace ge
