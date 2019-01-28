#include "ge/ReplServer.hpp"

#include "bl/vector.hpp"
#include "ge/Engine.hpp"
#include "ge/Tokenizer.hpp"
#include "sys/fiber.hpp"
#include "util/string.hpp"

namespace ge {

using namespace sys::io;
using namespace sys;
using sys::Fiber;

namespace {

enum TokenizeResult
{
    TokenizeOk,
    TokenizeFailed,
    TokenizeIncomplete
};

enum ParsingState
{
    ParsingGotStatement,
    ParsingYield,
    ParsingStop
};

struct Client
{
    int id;
    Fiber parser_fiber;
    Fiber &client_fiber;
    ParsingState state;
    HandleStream hstream;
    CooperativeInStream in_stream;
    ParseState parse_state;

    bl::vector<CommandArg> statement;

    Client(int _id, Handle h);
    ~Client();

    bool handle(Engine & /*e*/);
    bool handleIO(Engine & /*e*/);

    void run_parser();
};

// void
// fiber_cleanup(Fiber *self, void *args)
// {
//     ON_DEBUG(INFO("fiber_cleanup()"));
//     fiber_switch(self, reinterpret_cast<Fiber *>(args));
// }

void
Client::run_parser()
{
    while (state != ParsingStop) {
        bool ok = tokenize(parse_state, statement);
        if (ok) {
            state = ParsingGotStatement;
        } else {
            state = ParsingYield;
            Fiber::switch_to(client_fiber);
            if (state == ParsingStop)
                break;
            skipStatement(parse_state);
            state = ParsingYield;
        }
        Fiber::switch_to(client_fiber);
    }
}

Client::Client(int _id, Handle h)
  : id(_id)
  , parser_fiber(Fiber::make([this]() { this->run_parser(); }))
  , client_fiber(sys::Fiber::toplevel())
  , state(ParsingYield)
  , hstream(std::move(h))
  , in_stream(&hstream, client_fiber, parser_fiber)
  , parse_state(in_stream, "")
{
    sys::io::ByteStream name;
    name << "<repl " << id << ">";
    parse_state.filename = name.str();
}

Client::~Client()
{
    ASSERT(!parser_fiber.is_alive());
}

bool
Client::handle(Engine &e)
{
    sys::io::OutStream &out = e.out();
    e.out(hstream);
    bool ok = handleIO(e);
    e.out(out);
    return ok;
}

bool
Client::handleIO(Engine &e)
{
    Fiber::switch_to(parser_fiber);
    switch (state) {
    case ParsingGotStatement:
        e.commandProcessor().execCommand(statement);
        statement.clear();
        break;
    case ParsingYield:
        break;
    case ParsingStop:
        break;
    }

    sys::io::StreamResult res = parse_state.in_state;
    return res == sys::io::StreamResult::Blocked ||
           res == sys::io::StreamResult::OK;
}

} // namespace

struct ReplServer::Data
{
    ReplServer &self;
    Socket server;
    bl::vector<bl::shared_ptr<Client>> clients;
    bool running{ false };
    int next_id{};
    Engine &engine;
    bl::shared_ptr<EventHandler<InputEvent>> io_handler;

    Data(ReplServer &self_,
         Engine &e,
         bl::shared_ptr<EventHandler<InputEvent>> _handler)
      : self(self_), engine(e), io_handler(std::move(_handler))
    {}
};

DECLARE_PIMPL_DEL(ReplServer)

ReplServer::ReplServer(Engine &e)
  : self(new Data(*this,
                  e,
                  makeEventHandler(*this, &ReplServer::handleInputEvent)))
{}

ReplServer::~ReplServer()
{
    if (self->running)
        shutdown();
}

bool
ReplServer::start(const IPAddr4 &listen_addr, uint16_t port)
{
    if (self->running) {
        ERR("already running");
        return false;
    }

    sys::io::SocketError err;
    auto opt_sock = listen(SP_TCP, listen_addr, port, SM_NONBLOCKING, err);
    if (!opt_sock) {
        ERR(string_concat("failed to open socket for listening: ", err));
        return false;
    }
    self->server = std::move(opt_sock).value();
    self->running = true;
    return true;
}

bool
ReplServer::running()
{
    return self->running;
}

const bl::shared_ptr<EventHandler<InputEvent>> &
ReplServer::ioHandler()
{
    return self->io_handler;
}

void
ReplServer::acceptClients()
{
    ASSERT(self->running);

    for (;;) {
        SocketError err;
        auto opt_hndl = accept(self->server, err);
        if (!opt_hndl)
            break;
        auto handle = std::move(opt_hndl).value();
        INFO("accepted client");
        if (elevate(handle, mode(handle) | HM_NONBLOCKING) != HandleError::OK) {
            ERR("couldnt set nonblocking handle mode, closing connection");
            sys::io::close(handle);
        } else {
            self->clients.push_back(
              bl::make_shared<Client>(self->next_id++, std::move(handle)));
        }
    }
}

void
ReplServer::handleClients()
{
    ASSERT(self->running);

    for (size_t i = 0; i < self->clients.size();) {
        if (!self->clients[i]->handle(self->engine)) {
            INFO("closing connection to client");
            self->clients.erase(self->clients.begin() + i);
        } else {
            ++i;
        }
    }
}

void
ReplServer::handleIO()
{
    acceptClients();
    handleClients();
}

void
ReplServer::handleInputEvent(const Event<InputEvent> & /*unused*/)
{
    handleIO();
}

void
ReplServer::shutdown()
{
    ASSERT(self->running);

    self->clients.clear();
    close(self->server);
}

} // namespace ge
