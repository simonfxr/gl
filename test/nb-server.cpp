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
    size_t buf_end{ BUF_SIZE };

    explicit Client(io::Handle h, int id)
      : stream(std::make_shared<io::HandleStream>(std::move(h))), id(id)
    {}
};

int
main()
{
    std::vector<Client> clients;
    int id = 0;

    sys::moduleInit();

    auto server_sock =
      io::listen(io::SP_TCP, io::IPA_LOCAL, 1337, io::SM_NONBLOCKING);
    if (!server_sock) {
        ERR("failed to start server");
        return 1;
    }
    auto server = std::move(server_sock).value();

    INFO("started server");

    for (;;) {

        for (;;) {
            auto res = io::accept(server);
            if (!res)
                break;
            auto handle = std::move(res).value();
            IGNORE_RESULT(
              io::elevate(handle, io::mode(handle) | io::HM_NONBLOCKING));
            auto &c = clients.emplace_back(std::move(handle), id);
            sys::io::stdout() << "accepted client " << c.id << "\n";
        }

        for (size_t i = 0; i < clients.size(); i++) {
            Client &c = clients[i];
            bool close = false;

            if (c.reading) {
                size_t size = c.buf_end - c.buf_pos;
                io::StreamResult err;
                std::tie(size, err) =
                  c.stream->read(std::span{ c.buffer + c.buf_pos, size });
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
                std::tie(size, err) =
                  c.stream->write(std::span{ c.buffer + c.buf_pos, size });
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
                  << "closing connection to client " << c.id << "\n";
                c.stream->close();
                clients.erase(clients.begin() + i);
                --i; // may underflow, okay
            }
        }
    }
}
