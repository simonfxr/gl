#ifndef GE_REPL_SERVER_HPP
#define GE_REPL_SERVER_HPP

#include "sys/io.hpp"

#include "ge/EngineEvents.hpp"
#include "ge/Event.hpp"

#include <bl/new.hpp>

namespace ge {

struct Engine;

struct GE_API ReplServer : bl::runtime_sized_base<ReplServer>
{
    static ReplServer &construct_at(void *, Engine &) noexcept;

    bool start(const sys::io::IPAddr4 &, uint16_t);
    bool running();

    void acceptClients();
    void handleClients();
    void handleIO();
    void handleInputEvent(const Event<InputEvent> &);

    void shutdown();

    const std::shared_ptr<EventHandler<InputEvent>> &ioHandler();

    BL_DECLARE_RUNTIME_SIZED(ReplServer, Impl);
};

} // namespace ge

#endif
