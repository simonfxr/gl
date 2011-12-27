#include "ge/ReplServer.hpp"
#include "ge/Tokenizer.hpp"
#include "ge/Engine.hpp"

#include <vector>
#include <sstream>

namespace ge {

using namespace sys::io;

namespace {

enum TokenizeResult {
    TokenizeOk,
    TokenizeFailed,
    TokenizeIncomplete
};

TokenizeResult tryTokenize(ParseState& state, RecordingStream& in, std::vector<ge::CommandArg>& args) {
    state.in = &in;
    in.reset();
    in.replay();
    
    defs::index len = args.size();
    ParseState activeState = state;
    bool ok = tokenize(activeState, args);
    
    if (!ok && activeState.in_state == sys::io::StreamBlocked) {
        args.erase(args.begin() + len, args.end());
        return TokenizeIncomplete;
    }

    state = activeState;

    if (ok) {
        in.clear();
        return TokenizeOk;
    } else {
        return TokenizeFailed;
    }
}

struct Client {
    HandleStream stream;
    RecordingStream recorder;
    
    int id;
    ParseState parse_state;
    bool skip_statement;

    std::vector<CommandArg> statement;
    
    Client() :
        recorder(stream),
        parse_state(recorder, "<repl>"),
        skip_statement(false)
        {}
};

void initClient(Client& c) {
    std::ostringstream name;
    name << "<repl " << c.id << ">";
    c.parse_state.filename = name.str();
}

bool handleClient(Engine& e, Client& c) {

    {
        c.recorder.record();
        size s = 1;
        char ch;
        sys::io::StreamResult res = c.recorder.read(s, &ch);
        
        if (s == 0) {
            if (res == sys::io::StreamBlocked)
                return true;
            else
                return false;
        }
    }

    if (c.skip_statement) {

        c.recorder.reset();
        c.recorder.replay();
        
        if (!skipStatement(c.parse_state)) {
            if (c.parse_state.in_state == sys::io::StreamBlocked)
                return true;
            else
                return false;
        }
        
        c.skip_statement = false;

        if (c.parse_state.in_state == sys::io::StreamBlocked)
            return true;
        if (c.parse_state.in_state != sys::io::StreamOK)
            return false;
    }

    TokenizeResult res = tryTokenize(c.parse_state, c.recorder, c.statement);

    switch (res) {
    case TokenizeOk:
        // INFO("parsed statement");
        // std::cout << "statement: ";
        // printArgs(c.statement);
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

    if (c.parse_state.in_state == sys::io::StreamOK ||
        c.parse_state.in_state == sys::io::StreamBlocked)
        return true;
    else
        return false;
}

void closeClient(Client& c) {
    c.stream.close();
    for (index i = 0; i < SIZE(c.statement.size()); ++i)
        c.statement[i].free();
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
                  &self->clients[self->clients.size() - 1].stream.handle) == SE_OK) {
        
        Client& c = self->clients[self->clients.size() - 1];
        c.id = self->next_id++;
        INFO("accepted client");
        if (elevate(c.stream.handle, mode(c.stream.handle) | HM_NONBLOCKING) != HE_OK) {
            ERR("couldnt set nonblocking handle mode, closing connection");
            c.stream.close();
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
