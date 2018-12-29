#include "err/err.hpp"

#include "ge/CommandProcessor.hpp"
#include "ge/Engine.hpp"
#include "ge/KeyBinding.hpp"
#include "ge/Tokenizer.hpp"

#include <iostream>
#include <sstream>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

using namespace ge;
using namespace defs;

static void
printArgs(const std::vector<CommandArg> &args);

void
printKeyCombo(const KeyBinding &bind)
{
    const char *sep = "[";
    for (defs::index i = 0; i < bind.size(); ++i) {
        std::cout << sep;
        sep = ", ";
        const Key &k = bind[i];
        char pre;
        switch (k.state) {
        case Up:
            pre = '!';
            break;
        case Down:
            pre = 0;
            break;
        case Pressed:
            pre = '+';
            break;
        case Released:
            pre = '-';
            break;
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

void
printQuot(const Quotation &q)
{
    if (q.size() == 0) {
        std::cout << " { } ";
        return;
    }

    std::cout << "{ " << std::endl;
    for (uint32 i = 0; i < q.size(); ++i)
        printArgs(q[i]);
    std::cout << "}" << std::endl;
}

static void
printArg(const ge::CommandArg &arg)
{
    switch (arg.type) {
    case String:
        std::cout << '"' << *arg.string << '"';
        break;
    case Integer:
        std::cout << arg.integer;
        break;
    case Number:
        std::cout << arg.number;
        break;
    case KeyCombo:
        printKeyCombo(*arg.keyBinding);
        break;
    case CommandRef:
        std::cout << '&' << *arg.command.name;
        if (arg.command.quotation != 0)
            printQuot(*arg.command.quotation);
        break;
    case VarRef:
        std::cout << '$' << *arg.var;
        break;
    default:
        std::cout << "invalid type: " << arg.type;
    }
}

static void
printArgs(const std::vector<CommandArg> &args)
{
    const char *sep = "";
    for (uint32 i = 0; i < args.size(); ++i) {
        std::cout << sep;
        printArg(args[i]);
        sep = " ";
    }
    std::cout << std::endl;
}

Input::State
readFile(int fd, size &count, char *bytes)
{
    size_t n = UNSIZE(count);
    ssize_t k = read(fd, static_cast<void *>(bytes), n);
    if (k > 0) {
        count = UNSIZE(k);
        return Input::OK;
    } else if (k == 0) {
        count = 0;
        return Input::Eof;
    } else {
        count = 0;
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            errno = 0;
            return Input::Blocked;
        } else {
            ERR(strerror(errno));
            errno = 0;
            return Input::Error;
        }
    }
}

bool
setNonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        flags = 0;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        ERR(strerror(errno));
        return false;
    }
    INFO("set non blocking");
    return true;
}

struct NonblockingFile : public Input
{
    int fd;
    NonblockingFile(int _fd) : fd(_fd) { setNonblocking(fd); }

    Input::State next(char &c)
    {
        size s = 1;
        return readFile(fd, s, &c);
    }

    void close() {}
};

struct BufferedInput : public Input
{
    bool from_buffer;
    std::vector<char> buffer;
    defs::index buffer_pos;
    Ref<Input> in;
    BufferedInput(const Ref<Input> &_in) : from_buffer(false), in(_in) {}
    Input::State next(char &);
    void close() { in->close(); }

    void readFromBuffer()
    {
        from_buffer = true;
        buffer_pos = 0;
    }

    void clearBuffer()
    {
        buffer_pos = 0;
        buffer.clear();
    }
};

Input::State
BufferedInput::next(char &c)
{
    if (from_buffer) {
        if (buffer_pos < SIZE(buffer.size())) {
            c = buffer[buffer_pos++];
            return Input::OK;
        } else {
            from_buffer = false;
        }
    }

    Input::State s = in->next(c);
    if (s == Input::OK)
        buffer.push_back(c);

    return s;
}

enum TokenizeResult
{
    TokenizeOk,
    TokenizeFailed,
    TokenizeIncomplete
};

TokenizeResult
tryTokenize(ParseState &state,
            Ref<BufferedInput> &in,
            std::vector<ge::CommandArg> &args)
{
    state.in = in;
    in->readFromBuffer();
    defs::index len = args.size();
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

int
main(int argc, char *argv[])
{
    ge::Engine eng;
    ge::CommandProcessor proc(eng);
    Ref<Input> in(new NonblockingFile(STDIN_FILENO));
    // Ref<Input> in(new IStreamInput<std::istream>(std::cin));
    Ref<BufferedInput> bufin(new BufferedInput(in));
    ge::ParseState state(bufin, "<interactive>");
    bool skip_statement = false;
    bool incomplete = false;

    while (state.in_state == Input::OK || state.in_state == Input::Blocked) {
        std::vector<ge::CommandArg> args;

        if (skip_statement) {
            if (!skipStatement(state)) {
                INFO("skipping");
                continue;
            }

            skip_statement = false;
            bufin->clearBuffer();
        }

        TokenizeResult res = tryTokenize(state, bufin, args);

        switch (res) {
        case TokenizeOk:

            incomplete = false;
            INFO("ok");
            std::cerr << "STATEMENT: ";
            printArgs(args);
            for (uint32 i = 0; i < args.size(); ++i)
                args[i].free();
            break;

        case TokenizeFailed:

            INFO("failed");
            skip_statement = true;
            incomplete = false;

            break;

        case TokenizeIncomplete:

            if (!incomplete) {
                INFO("incomplete");
                incomplete = true;
            }

            break;
        }
    }

    return 0;
}
