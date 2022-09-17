#include "ge/ReplServer.hpp"

#include "ge/Engine.hpp"
#include "ge/Tokenizer.hpp"
#include "sys/fiber.hpp"
#include "util/string.hpp"

#include <utility>
#include <vector>

namespace ge {

using namespace sys::io;

namespace {

enum class ParsingState : uint8_t
{
    GotStatement,
    Yield,
    Stop,
};

struct Client : private NonCopyMoveable
{
    DISABLE_COPY_MOVE_MEMBERS(Client);

    Fiber parser_fiber{};
    Fiber *client_fiber{};
    HandleStream hstream;
    CooperativeInStream in_stream;
    ParserState parser;
    ParsingState state = ParsingState::Yield;
    int id;

    std::vector<CommandArg> statement;

    Client(int _id, Handle h);
    ~Client();

    bool handle(Engine & /*e*/);
    bool handleIO(Engine & /*e*/);
};

void
fiber_cleanup(Fiber *self, void *args)
{
    ON_DEBUG(INFO("fiber_cleanup()"));
    fiber_switch(self, static_cast<Fiber *>(args));
}

void
run_parser_fiber(void *args)
{
    auto *c = static_cast<Client *>(args);
    while (c->state != ParsingState::Stop) {
        bool ok = tokenize(c->parser, c->statement);
        if (ok) {
            c->state = ParsingState::GotStatement;
        } else {
            c->state = ParsingState::Yield;
            fiber_switch(&c->parser_fiber, c->client_fiber);
            if (c->state == ParsingState::Stop)
                break;
            skipStatement(c->parser);
            c->state = ParsingState::Yield;
        }
        fiber_switch(&c->parser_fiber, c->client_fiber);
    }
}

Client::Client(int _id, Handle h)
  : client_fiber(sys::fiber::toplevel())
  , hstream(std::move(h))
  , in_stream(&hstream, client_fiber, &parser_fiber)
  , parser(in_stream, "")
  , id(_id)
{
    (void) fiber_alloc(
      &parser_fiber, 65536, fiber_cleanup, &client_fiber, true);
    Client *parser_args = this;
    fiber_push_return(
      &parser_fiber,
      run_parser_fiber,
      static_cast<void *>(&parser_args),
      // we really pass a pointer here
      sizeof parser_args /* NOLINT(bugprone-sizeof-expression)*/);
    sys::io::ByteStream name;
    name << "<repl " << id << ">";
    parser.filename = name.str();
}

Client::~Client()
{
    INFO("invoking ~Client()");
    if (fiber_is_alive(&parser_fiber)) {
        state = ParsingState::Stop;
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
    case ParsingState::GotStatement:
        e.commandProcessor().execCommand(std::span(statement));
        statement.clear();
        break;
    case ParsingState::Yield:
    case ParsingState::Stop:
        break;
    }

    sys::io::StreamResult res = parser.in_state;
    return res == sys::io::StreamResult::Blocked ||
           res == sys::io::StreamResult::OK;
}

} // namespace

struct ReplServer::Data
{
    ReplServer &self;
    Socket server;
    std::vector<std::shared_ptr<Client>> clients;
    bool running{ false };
    int next_id{};
    Engine &engine;
    std::shared_ptr<EventHandler<InputEvent>> io_handler;

    Data(ReplServer &self_,
         Engine &e,
         std::shared_ptr<EventHandler<InputEvent>> _handler)
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

    auto sock = listen(SP_TCP, listen_addr, port, SM_NONBLOCKING);
    if (!sock) {
        ERR(
          string_concat("failed to open socket for listening: ", sock.error()));
        return false;
    }
    self->server = std::move(sock).value();
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

    for (;;) {
        auto opt_hndl = accept(self->server);
        if (!opt_hndl)
            break;
        auto handle = std::move(opt_hndl).value();
        INFO("accepted client");
        if (elevate(handle, mode(handle) | HM_NONBLOCKING) != HandleError::OK) {
            ERR("couldnt set nonblocking handle mode, closing connection");
            sys::io::close(handle);
        } else {
            self->clients.push_back(
              std::make_shared<Client>(self->next_id++, std::move(handle)));
        }
    }
}

void
ReplServer::handleClients()
{
    ASSERT(self->running);

    for (auto it = self->clients.begin(); it != self->clients.end();) {
        if (!(*it)->handle(self->engine)) {
            INFO("closing connection to client");
            self->clients.erase(it);
        } else {
            ++it;
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
