#include "ge/ReplServer.hpp"
#include "ge/Tokenizer.hpp"
#include "ge/Engine.hpp"

#include <vector>
#include <sstream>

namespace ge {

using namespace sys::io;

namespace {

const size HANDLE_BUF_SIZE = 4096;

struct HandleInput : public Input {
    Handle handle;
    char buf[HANDLE_BUF_SIZE];
    index buf_pos;
    index buf_end;
    bool eof;
    
    Input::State next(char&);
    void close() {}
};

Input::State HandleInput::next(char& c) {
    if (buf_pos < buf_end) {
        c = buf[buf_pos++];
        return Input::OK;
    } else if (eof) {
        return Input::Eof;
    } else {
        size s = HANDLE_BUF_SIZE;
        HandleError err = read(handle, s, buf);
        switch (err) {
        case HE_OK:
            buf_pos = 0;
            buf_end = s;
            c = buf[buf_pos++];
            return Input::OK;
        case HE_BLOCKED: return Input::Blocked;
        case HE_EOF:
            eof = true;
            return Input::Eof;
        case HE_UNKNOWN:
            return Input::Error;
        default:
            ASSERT_FAIL();
        }
    }
}

struct BufferedInput : public Input {
    std::vector<char> buffer;
    defs::index buffer_pos;
    Ref<Input> in;
    
    BufferedInput(const Ref<Input>& _in) :
        buffer_pos(0),
        in(_in)
        {}
    
    Input::State next(char&);
    void close() { in->close(); }
};

Input::State BufferedInput::next(char& c) {
    
    if (buffer_pos < SIZE(buffer.size())) {
        c = buffer[buffer_pos++];
        return Input::OK;
    }

    ASSERT(buffer_pos == SIZE(buffer.size()));
    
    Input::State s = in->next(c);
    if (s == Input::OK) {
        std::cerr << "next: '" << c << "'" << std::endl;
        buffer.push_back(c);
        ++buffer_pos;
    }
    
    return s;
}

enum TokenizeResult {
    TokenizeOk,
    TokenizeFailed,
    TokenizeIncomplete
};

static void printArgs(const std::vector<CommandArg>& args);

void printKeyCombo(const KeyBinding& bind) {
    const char *sep = "[";
    for (defs::index i = 0; i < bind.size(); ++i) {
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

TokenizeResult tryTokenize(ParseState& state, Ref<BufferedInput>& in, std::vector<ge::CommandArg>& args) {
    state.in = in;
    in->buffer_pos = 0;
    defs::index len = args.size();
    ParseState activeState = state;
    bool ok = tokenize(activeState, args);
    
    if (!ok && activeState.in_state == Input::Blocked) {
        args.erase(args.begin() + len, args.end());
        return TokenizeIncomplete;
    }

    state = activeState;

    if (ok) {
        ASSERT(in->buffer_pos == SIZE(in->buffer.size()));
        in->buffer_pos = 0;
        in->buffer.clear();
        return TokenizeOk;
    } else {
        return TokenizeFailed;
    }
}

struct Client {
    Ref<HandleInput> input;
    Ref<BufferedInput> buf_input;
    
    int id;
    ParseState parse_state;
    bool skip_statement;

    std::vector<CommandArg> statement;
    
    Client() :
        input(new HandleInput),
        buf_input(new BufferedInput(input)),
        parse_state(Ref<Input>(), "<repl>"),
        skip_statement(false)
        {}
};

void initClient(Client& c) {
    std::ostringstream name;
    name << "<repl " << c.id << ">";
    c.parse_state.filename = name.str();
}

bool handleClient(Engine& e, Client& c) {

    if (c.skip_statement) {
        
        if (!skipStatement(c.parse_state)) {
            if (c.parse_state.in_state == Input::Blocked)
                return true;
            else
                return false;
        }
        
        c.skip_statement = false;

        if (c.parse_state.in_state == Input::Blocked)
            return true;
        if (c.parse_state.in_state == Input::Error)
            return false;
    }

    TokenizeResult res = tryTokenize(c.parse_state, c.buf_input, c.statement);

    switch (res) {
    case TokenizeOk:
        INFO("parsed statement");
        std::cout << "statement: ";
        printArgs(c.statement);
        e.commandProcessor().execCommand(c.statement);
        for (index i = 0; i < SIZE(c.statement.size()); ++i)
            c.statement[i].free();
        c.statement.clear();
        break;
    case TokenizeIncomplete:
        break;
    case TokenizeFailed:
        c.skip_statement = true;
        break;
    }

    if (c.parse_state.in_state == Input::Eof ||
        c.parse_state.in_state == Input::Error)
        return false;
    else
        return true;
}

void closeClient(Client& c) {
    close(c.input->handle);
    for (index i = 0; i < SIZE(c.statement.size()); ++i)
        c.statement[i].free();
    c.statement.clear();
}

} // namespace anon

struct ReplServer::Data {
    Socket server;
    std::vector<Client> clients;
    bool running;
    int next_id;
    Engine& engine;
    Ref<EventHandler<InputEvent> > io_handler;

    Data(Engine& e, const Ref<EventHandler<InputEvent> >& _handler) :
        running(false),
        next_id(0),
        engine(e),
        io_handler(_handler)
        {}
};

ReplServer::ReplServer(Engine& e) :
    self(new Data(e, makeEventHandler(this, &ReplServer::handleInputEvent)))
{}

ReplServer::~ReplServer() {
    if (self->running)
        shutdown();
    delete self;
}

bool ReplServer::start(const IPAddr& listen_addr, uint16 port) {
    
    if (self->running) {
        ERR("already running");
        return false;
    }
    
    if (listen(SP_TCP, listen_addr, port, SM_NONBLOCKING, &self->server) != SE_OK)
        return false;
    self->clients.push_back(Client());
    self->running = true;
    return true;
}

bool ReplServer::running() {
    return self->running;
}

Ref<EventHandler<InputEvent> >& ReplServer::ioHandler() {
    return self->io_handler;
}

void ReplServer::acceptClients() {
    ASSERT(self->running);
    
    while (accept(self->server,
                  &self->clients[self->clients.size() - 1].input->handle) == SE_OK) {
        
        Client& c = self->clients[self->clients.size() - 1];
        c.id = self->next_id++;
        INFO("accepted client");
        if (elevate(c.input->handle, mode(c.input->handle) | HM_NONBLOCKING) != HE_OK) {
            ERR("couldnt set nonblocking handle mode, closing connection");
            close(c.input->handle);
        } else {
            initClient(c);
            self->clients.push_back(Client());
        }
    }
}

void ReplServer::handleClients() {

    ASSERT(self->running);
    
    for (index i = 0; i < SIZE(self->clients.size() - 1); ++i) {
        if (!handleClient(self->engine, self->clients[i])) {
            INFO("closing connection to client");
            closeClient(self->clients[i]);
            self->clients.erase(self->clients.begin() + i);
        }
    }
}

void ReplServer::handleIO() {
    acceptClients();
    handleClients();
}

void ReplServer::handleInputEvent(const Event<InputEvent>&) {
    handleIO();
}

void ReplServer::shutdown() {

    ASSERT(self->running);
    
    for (index i = 0; i < SIZE(self->clients.size() - 1); ++i)
        closeClient(self->clients[i]);

    self->clients.clear();
    close(self->server);
}

} // namespace ge
