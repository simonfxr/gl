#include "data/Ref.hpp"
#include "err/err.hpp"
#include "sys/io.hpp"
#include "sys/io/Stream.hpp"
#include "sys/sys.hpp"

#include <vector>

using namespace defs;
using namespace sys;

static const size BUF_SIZE = 4096;

struct Client
{
    Ref<io::HandleStream> stream;
    int id;
    bool reading;
    char buffer[BUF_SIZE];
    index buf_pos;
    index buf_end;

    Client()
      : stream(new io::HandleStream),
        id(0),
        reading(true),
        buf_pos(0),
        buf_end(BUF_SIZE)
    {
    }
};

int
main(void)
{

    io::Socket server;
    std::vector<Client> clients;
    clients.push_back(Client());
    int id = 0;

    sys::moduleInit();

    if (io::listen(io::SP_TCP, io::IPA_LOCAL(), 1337, io::SM_NONBLOCKING,
                   &server) != io::SE_OK) {
        ERR("failed to start server");
        return 1;
    }

    INFO("started server");

    for (;;) {

        while (
          io::accept(server, &clients[clients.size() - 1].stream->handle) ==
          io::SE_OK) {
            Client &c = clients[clients.size() - 1];
            c.id = id++;
            sys::io::stdout() << "accepted client " << c.id << sys::io::endl;
            io::elevate(c.stream->handle,
                        io::mode(c.stream->handle) | io::HM_NONBLOCKING);
            clients.push_back(Client());
        }

        for (index i = 0; i < SIZE(clients.size()) - 1; ++i) {
            Client &c = clients[i];
            bool close = false;

            if (c.reading) {
                index size = c.buf_end - c.buf_pos;
                io::StreamResult err;
                err = c.stream->read(size, c.buffer + c.buf_pos);
                c.buf_pos += size;
                if (c.buf_pos == c.buf_end) {
                    c.reading = false;
                    c.buf_pos = 0;
                }

                if (err == io::StreamResult::EOF ||
                    err == io::StreamResult::Error) {
                    close = true;
                } else if (err == io::StreamResult::Blocked && c.buf_pos > 0) {
                    c.reading = false;
                    c.buf_end = c.buf_pos;
                    c.buf_pos = 0;
                }

            } else {
                index size = c.buf_end - c.buf_pos;
                io::StreamResult err;
                err = c.stream->write(size, c.buffer + c.buf_pos);
                c.buf_pos += size;

                if (c.buf_pos == c.buf_end) {
                    c.reading = true;
                    c.buf_pos = 0;
                    c.buf_end = BUF_SIZE;
                    err = c.stream->flush();
                }

                if (err == io::StreamResult::EOF ||
                    err == io::StreamResult::Error)
                    close = true;
            }

            if (close) {
                sys::io::stdout() << "closing connection to client " << c.id
                                  << sys::io::endl;
                c.stream->close();
                clients.erase(clients.begin() + i);
            }
        }
    }

    sys::moduleExit();
}
