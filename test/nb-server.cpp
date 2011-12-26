#include "sys/io.hpp"
#include "err/err.hpp"

#include <vector>

using namespace defs;
using namespace sys;

const size BUF_SIZE = 4096;

struct Client {
    io::Handle handle;
    int id;
    char buffer[BUF_SIZE];
    bool reading;
    index buf_pos;
    index buf_end;

    Client() :
        id(0),
        reading(true),
        buf_pos(0),
        buf_end(BUF_SIZE)
        {}
};

int main(void) {

    io::Socket server;
    std::vector<Client> clients;
    clients.push_back(Client());
    int id = 0;

    if (io::listen(io::SP_TCP, io::IPA_LOCAL, 1337, io::SM_NONBLOCKING, &server) != io::SE_OK) {
        ERR("failed to start server");
        return 1;
    }

    INFO("started server");

    for (;;) {

        while (io::accept(server, &clients[clients.size() - 1].handle) == io::SE_OK) {
            Client& c = clients[clients.size() - 1];
            c.id = id++;
            std::cerr << "accepted client " << c.id << std::endl;
            io::elevate(c.handle, io::mode(c.handle) | io::HM_NONBLOCKING);
            clients.push_back(Client());

        }

        for (index i = 0; i < SIZE(clients.size()) - 1; ++i) {
            Client& c = clients[i];
            bool close = false;
            
            if (c.reading) {
                index size = c.buf_end - c.buf_pos;
                io::HandleError err;
                err = io::read(c.handle, size, c.buffer + c.buf_pos);
                if (err == io::HE_OK) {
                    c.buf_pos += size;
                    if (c.buf_pos == c.buf_end) {
                        c.reading = false;
                        c.buf_pos = 0;
                    }                        
                } else if (err == io::HE_EOF) {
                    close = true;
                } else if (err == io::HE_BLOCKED && c.buf_pos > 0) {
                    c.reading = false;
                    c.buf_end = c.buf_pos;
                    c.buf_pos = 0;
                }
            } else {
                index size = c.buf_end - c.buf_pos;
                io::HandleError err;
                err = io::write(c.handle, size, c.buffer + c.buf_pos);
                if (err == io::HE_OK) {
                    c.buf_pos += size;
                    if (c.buf_pos == c.buf_end) {
                        c.reading = true;
                        c.buf_pos = 0;
                        c.buf_end = BUF_SIZE;
                    }
                } else if (err == io::HE_EOF) {
                    close = true;
                }
            }

            if (close) {
                std::cerr << "closing connection to client " << c.id << std::endl;
                io::close(c.handle);
                clients.erase(clients.begin() + i);
            }
        }
    }
}
