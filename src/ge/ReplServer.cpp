#include "ge/ReplServer.hpp"
#include "ge/Engine.hpp"
#include "ge/Tokenizer.hpp"

#include "sys/fiber.hpp"

#include <utility>
#include <vector>

namespace ge {

using namespace sys::io;

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
    Fiber *client_fiber;
    ParsingState state;
    HandleStream hstream;
    CooperativeInStream in_stream;
    ParseState parse_state;

    std::vector<CommandArg> statement;

    Client(int _id, Handle h);
    ~Client();

    bool handle(Engine & /*e*/);
    bool handleIO(Engine & /*e*/);
};

struct ParserArgs
{
    Client *client;
};

void
run_parser_fiber(void *args0)
{
    auto *args = reinterpret_cast<ParserArgs *>(args0);
    Client *c = args->client;
    while (c->state != ParsingStop) {
        bool ok = tokenize(c->parse_state, c->statement);
        if (ok) {
            c->state = ParsingGotStatement;
        } else {
            c->state = ParsingYield;
            fiber_switch(&c->parser_fiber, c->client_fiber);
            if (c->state == ParsingStop)
                break;
            skipStatement(c->parse_state);
            c->state = ParsingYield;
        }
        fiber_switch(&c->parser_fiber, c->client_fiber);
    }

    fiber_set_alive(&c->parser_fiber, 0);
    INFO("run_parser_fiber returns");
    fiber_switch(&c->parser_fiber, c->client_fiber);
}

Client::Client(int _id, Handle h)
  : id(_id)
  , parser_fiber()
  , client_fiber(sys::fiber::toplevel())
  , state(ParsingYield)
  , hstream(h)
  , in_stream(&hstream, client_fiber, &parser_fiber)
  , parse_state(in_stream, "")
{
    fiber_alloc(&parser_fiber, 65536, 2);
    ParserArgs args{};
    args.client = this;
    fiber_push_return(
      &parser_fiber, run_parser_fiber, static_cast<void *>(&args), sizeof args);
    sys::io::ByteStream name;
    name << "<repl " << id << ">";
    parse_state.filename = name.str();
}

Client::~Client()
{
    INFO("invoking ~Client()");
    if (fiber_is_alive(&parser_fiber)) {
        state = ParsingStop;
        fiber_switch(client_fiber, &parser_fiber);
    }
    ASSERT(!fiber_is_alive(&parser_fiber));
    fiber_destroy(&parser_fiber);
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
    fiber_switch(client_fiber, &parser_fiber);
    switch (state) {
    case ParsingGotStatement:
        e.commandProcessor().execCommand(statement);
        for (auto &s : statement)
            s.free();
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
    ReplServer *const self;
    Socket server;
    std::vector<std::shared_ptr<Client>> clients;
    bool running{ false };
    int next_id{};
    Engine &engine;
    std::shared_ptr<EventHandler<InputEvent>> io_handler;

    Data(ReplServer *self_,
         Engine &e,
         std::shared_ptr<EventHandler<InputEvent>> _handler)
      : self(self_), engine(e), io_handler(std::move(_handler))
    {}

    ~Data()
    {
        if (running)
            self->shutdown();
    }
};

DECLARE_PIMPL_DEL(ReplServer)

ReplServer::ReplServer(Engine &e)
  : self(
      new Data(this, e, makeEventHandler(this, &ReplServer::handleInputEvent)))
{}

bool
ReplServer::start(const IPAddr4 &listen_addr, uint16_t port)
{

    if (self->running) {
        ERR("already running");
        return false;
    }

    if (listen(SP_TCP, listen_addr, port, SM_NONBLOCKING, &self->server) !=
        SE_OK)
        return false;
    self->running = true;
    return true;
}

bool
ReplServer::running()
{
    return self->running;
}

const std::shared_ptr<EventHandler<InputEvent>> &
ReplServer::ioHandler()
{
    return self->io_handler;
}

void
ReplServer::acceptClients()
{
    ASSERT(self->running);

    Handle handle;
    while (accept(self->server, &handle) == SE_OK) {
        INFO("accepted client");
        if (elevate(handle, mode(handle) | HM_NONBLOCKING) != HE_OK) {
            ERR("couldnt set nonblocking handle mode, closing connection");
            sys::io::close(handle);
        } else {
            self->clients.push_back(
              std::make_shared<Client>(self->next_id++, handle));
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
