#ifndef GE_REPL_SERVER_HPP
#define GE_REPL_SERVER_HPP

#include "sys/io.hpp"

#include "ge/Event.hpp"
#include "ge/EngineEvents.hpp"

namespace ge {

struct Engine;

using namespace defs;

struct ReplServer {

    ReplServer(Engine&);
    ~ReplServer();

    bool start(const sys::io::IPAddr&, uint16);
    bool running();

    void acceptClients();
    void handleClients();
    void handleIO();
    void handleInputEvent(const Event<InputEvent>&);
    
    void shutdown();
    
    Ref<EventHandler<InputEvent> >& ioHandler();

private:
    struct Data;
    Data * const self;

    ReplServer(const ReplServer&);
    ReplServer& operator =(const ReplServer&);
};

} // namespace ge

#endif
