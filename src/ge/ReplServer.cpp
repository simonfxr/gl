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

struct Client
{
    BL_DISABLE_COPY_MOVE(Client);

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
        auto ok = tokenize(c->parser, c->statement);
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
  , hstream(bl::move(h))
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
    auto &out = e.out();
    e.out(hstream);
    auto ok = handleIO(e);
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

    auto res = parser.in_state;
    return res == sys::io::StreamResult::Blocked ||
           res == sys::io::StreamResult::OK;
}

} // namespace

struct ReplServer::Impl : public ReplServer
{
    Socket server;
    std::vector<std::shared_ptr<Client>> clients;
    bool running{ false };
    int next_id{};
    Engine &engine;
    std::shared_ptr<EventHandler<InputEvent>> io_handler;

    Impl(Engine &e)
      : engine(e)
      , io_handler(makeEventHandler(*this, &ReplServer::handleInputEvent))
    {}

    ~Impl()
    {
        if (running)
            shutdown();
    }
};

BL_DEFINE_RUNTIME_SIZED(ReplServer, ReplServer::Impl);

ReplServer &
ReplServer::construct_at(void *p, Engine &e) noexcept
{
    return construct_impl(p, e);
}

bool
ReplServer::start(const IPAddr4 &listen_addr, uint16_t port)
{

    if (m().running) {
        ERR("already running");
        return false;
    }

    auto sock = listen(SP_TCP, listen_addr, port, SM_NONBLOCKING);
    if (!sock) {
        ERR(
          string_concat("failed to open socket for listening: ", sock.error()));
        return false;
    }
    m().server = bl::move(sock).value();
    m().running = true;
    return true;
}

bool
ReplServer::running()
{
    return m().running;
}

const std::shared_ptr<EventHandler<InputEvent>> &
ReplServer::ioHandler()
{
    return m().io_handler;
}

void
ReplServer::acceptClients()
{
    ASSERT(m().running);

    for (;;) {
        auto opt_hndl = accept(m().server);
        if (!opt_hndl)
            break;
        auto handle = bl::move(opt_hndl).value();
        INFO("accepted client");
        if (elevate(handle, mode(handle) | HM_NONBLOCKING) != HandleError::OK) {
            ERR("couldnt set nonblocking handle mode, closing connection");
            sys::io::close(handle);
        } else {
            m().clients.push_back(
              std::make_shared<Client>(m().next_id++, bl::move(handle)));
        }
    }
}

void
ReplServer::handleClients()
{
    ASSERT(m().running);

    for (auto it = m().clients.begin(); it != m().clients.end();) {
        if (!(*it)->handle(m().engine)) {
            INFO("closing connection to client");
            m().clients.erase(it);
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
    ASSERT(m().running);

    m().clients.clear();
    close(m().server);
}

} // namespace ge
