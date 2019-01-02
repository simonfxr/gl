#ifndef GE_REPL_SERVER_HPP
#define GE_REPL_SERVER_HPP

#include "sys/io.hpp"

#include "ge/EngineEvents.hpp"
#include "ge/Event.hpp"

namespace ge {

struct Engine;

using namespace defs;

struct GE_API ReplServer
{

    ReplServer(Engine &);

    bool start(const sys::io::IPAddr4 &, uint16);
    bool running();

    void acceptClients();
    void handleClients();
    void handleIO();
    void handleInputEvent(const Event<InputEvent> &);

    void shutdown();

    const std::shared_ptr<EventHandler<InputEvent>> &ioHandler();

private:
    struct Data;
    struct DataDeleter
    {
        void operator()(Data *) noexcept;
    };
    const std::unique_ptr<Data, DataDeleter> self;
};

} // namespace ge

#endif
