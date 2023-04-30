#include "err/err.hpp"
#include "sys/io.hpp"
#include "sys/io/Stream.hpp"
#include "sys/sys.hpp"

#include <bl/tuple.hpp>
#include <bl/vec.hpp>

#include <memory>

using namespace sys;
using namespace bl::literals;

static const size_t BUF_SIZE = 4096;

struct Client
{
    std::shared_ptr<io::HandleStream> stream;
    int id{};
    bool reading{ true };
    bl::byte buffer[BUF_SIZE]{};
    size_t buf_pos{};
    size_t buf_end{ BUF_SIZE };

    explicit Client(io::Handle h, int id)
      : stream(std::make_shared<io::HandleStream>(bl::move(h))), id(id)
    {}
};

int
main()
{
    bl::vec<Client> clients;
    int id = 0;

    sys::moduleInit();

    auto server_sock =
      io::listen(io::SP_TCP, io::IPA_LOCAL, 1337, io::SM_NONBLOCKING);
    if (!server_sock) {
        ERR("failed to start server");
        return 1;
    }
    auto server = bl::move(server_sock).value();

    INFO("started server");

    for (;;) {

        for (;;) {
            auto res = io::accept(server);
            if (!res)
                break;
            auto handle = bl::move(res).value();
            IGNORE_RESULT(
              io::elevate(handle, handle.mode() | io::HM_NONBLOCKING));
            auto &c = clients.emplace_back(bl::move(handle), id);
            sys::io::stdout() << "accepted client "_k << c.id << "\n"_k;
        }

        for (size_t i = 0; i < clients.size(); i++) {
            Client &c = clients[i];
            bool close = false;

            if (c.reading) {
                size_t size = c.buf_end - c.buf_pos;
                io::StreamResult err;
                bl::tie(size, err) = sys::io::read(
                  *c.stream, bl::span{ c.buffer + c.buf_pos, size });
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
                bl::tie(size, err) = sys::io::write(
                  *c.stream, bl::span{ c.buffer + c.buf_pos, size });
                c.buf_pos += size;

                if (c.buf_pos == c.buf_end) {
                    c.reading = true;
                    c.buf_pos = 0;
                    c.buf_end = BUF_SIZE;
                    err = sys::io::flush(*c.stream);
                }

                if (err == io::StreamResult::EOF ||
                    err == io::StreamResult::Error)
                    close = true;
            }

            if (close) {
                sys::io::stdout()
                  << "closing connection to client "_k << c.id << "\n"_k;
                sys::io::close(*c.stream);
                clients.erase(clients.begin() + i);
                --i; // may underflow, okay
            }
        }
    }
}
