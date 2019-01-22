#ifndef GE_REPL_SERVER_HPP
#define GE_REPL_SERVER_HPP

#include "sys/io.hpp"

#include "ge/EngineEvents.hpp"
#include "ge/Event.hpp"
#include "pp/pimpl.hpp"

namespace ge {

struct Engine;

struct GE_API ReplServer
{
    ReplServer(Engine &);
    ~ReplServer();

    bool start(const sys::io::IPAddr4 &, uint16_t);
    bool running();

    void acceptClients();
    void handleClients();
    void handleIO();
    void handleInputEvent(const Event<InputEvent> &);

    void shutdown();

    const bl::shared_ptr<EventHandler<InputEvent>> &ioHandler();

private:
    DECLARE_PIMPL(GE_API, self);
};

} // namespace ge

#endif
