#include "err/err.hpp"
#include "sys/io.hpp"
#include "sys/io/Stream.hpp"
#include "sys/sys.hpp"

#include <memory>
#include <vector>

using namespace sys;

static const size_t BUF_SIZE = 4096;

struct Client
{
    std::shared_ptr<io::HandleStream> stream;
    int id{};
    bool reading{ true };
    char buffer[BUF_SIZE]{};
    size_t buf_pos{};
    size_t buf_end;

    Client() : stream(new io::HandleStream), buf_end(BUF_SIZE) {}
};

int
main()
{

    io::Socket server;
    std::vector<Client> clients;
    clients.emplace_back();
    int id = 0;

    sys::moduleInit();

    if (io::listen(
          io::SP_TCP, io::IPA_LOCAL(), 1337, io::SM_NONBLOCKING, &server) !=
        io::SE_OK) {
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
            clients.emplace_back();
        }

        for (size_t i = 0; i < clients.size() - 1; ++i) {
            Client &c = clients[i];
            bool close = false;

            if (c.reading) {
                size_t size = c.buf_end - c.buf_pos;
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
                size_t size = c.buf_end - c.buf_pos;
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
                sys::io::stdout()
                  << "closing connection to client " << c.id << sys::io::endl;
                c.stream->close();
                clients.erase(clients.begin() + i);
            }
        }
    }

    sys::moduleExit();
}
