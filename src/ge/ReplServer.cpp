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

struct ClientStream {
    HandleStream conn;
    RecordingStream recorder;

    ClientStream(const Handle& h) :
        conn(h),
        recorder(conn)
        {}
};

struct Client {
    int id;
    Ref<ClientStream> stream;
    ParseState parse_state;
    bool skip_statement;
    
    Client(int _id, Handle h);
    bool handle(Engine&);
    bool handleIO(Engine&);
};

Client::Client(int _id, Handle h) :
    id(_id),
    stream(new ClientStream(h)),
    parse_state(stream->recorder, ""),
    skip_statement(false)
{
    std::ostringstream name;
    name << "<repl " << id << ">";
    parse_state.filename = name.str();
}

bool Client::handle(Engine& e) {
    sys::io::OutStream& out = e.out();
    e.out(stream->conn);
    bool ok = handleIO(e);
    e.out(out);
    return ok;
}

bool Client::handleIO(Engine& e) {
    sys::io::StreamResult res = stream->conn.flush();
    if (res == sys::io::StreamBlocked)
        return true;
    if (res != sys::io::StreamOK)
        return false;

    {
        stream->recorder.record();
        size s = 1;
        char ch;
        sys::io::StreamResult res = stream->recorder.read(s, &ch);
        
        if (s == 0) {
            if (res == sys::io::StreamBlocked)
                return true;
            else
                return false;
        }
    }

    stream->recorder.reset();
    stream->recorder.replay();

    if (skip_statement) {
        
        if (!skipStatement(parse_state)) {
            if (parse_state.in_state == sys::io::StreamBlocked)
                return true;
            else
                return false;
        }
        
        skip_statement = false;

        if (parse_state.in_state == sys::io::StreamBlocked)
            return true;
        
        if (parse_state.in_state != sys::io::StreamOK)
            return false;
    }

    std::vector<CommandArg> statement;
    bool parsed;
    bool incomplete;

    {
        ParseState activeState = parse_state;
        parsed  = tokenize(activeState, statement);
        if (!parsed && activeState.in_state == sys::io::StreamBlocked) {
            incomplete = true;
        } else {
            parse_state = activeState;
            incomplete = false;
        }
    }

    if (parsed)
        e.commandProcessor().execCommand(statement);

    if (parsed || !incomplete)
        stream->recorder.clear();

    if (!parsed && !incomplete)
        skip_statement = true;
       
    for (index i = 0; i < SIZE(statement.size()); ++i)
        statement[i].free();

    if (parse_state.in_state != sys::io::StreamOK &&
        parse_state.in_state != sys::io::StreamBlocked)
        return false;

    res = stream->conn.flush();
    if (res != sys::io::StreamOK &&
        res != sys::io::StreamBlocked)
        return false;
    else
        return true;
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

bool ReplServer::start(const IPAddr4& listen_addr, uint16 port) {
    
    if (self->running) {
        ERR("already running");
        return false;
    }
    
    if (listen(SP_TCP, listen_addr, port, SM_NONBLOCKING, &self->server) != SE_OK)
        return false;
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

    Handle handle;
    while (accept(self->server, &handle) == SE_OK) {
        INFO("accepted client");
        if (elevate(handle, mode(handle) | HM_NONBLOCKING) != HE_OK) {
            ERR("couldnt set nonblocking handle mode, closing connection");
            sys::io::close(handle);
        } else {
            self->clients.push_back(Client(self->next_id++, handle));
        }
    }
}

void ReplServer::handleClients() {
    ASSERT(self->running);
    
    for (index i = 0; i < SIZE(self->clients.size()); ++i) {
        if (!self->clients[i].handle(self->engine)) {
            INFO("closing connection to client");
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

    self->clients.clear();
    close(self->server);
}

} // namespace ge
