#include "connection.hpp"

#include "../http/response.hpp"

#include <cerrno>
#include <cstddef>
#include <poll.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>

namespace {

constexpr int kIdleTimeoutMs = 5000;

}

Connection::Connection(Socket socket, const Router& router)
    : socket_(std::move(socket)), router_(router) {}

void Connection::run() {
    while (true) {
        ParseStatus status = parser_.parse();

        if (status == ParseStatus::Complete) {
            const Request& request = parser_.request();
            bool keep_alive = request.keep_alive;
            Response response = router_.route(request);
            std::string bytes = serialize_response(response, keep_alive);
            if (!write_all(bytes)) break;
            parser_.discard_consumed();
            if (!keep_alive) break;
            continue;
        }

        if (status == ParseStatus::Error) {
            Response response;
            response.status = 400;
            write_all(serialize_response(response, false));
            break;
        }

        if (!read_more()) break;
    }
}

bool Connection::read_more() {
    pollfd poll_fd{};
    poll_fd.fd = socket_.get();
    poll_fd.events = POLLIN;
    int result = ::poll(&poll_fd, 1, kIdleTimeoutMs);
    if (result <= 0) return false;
    if ((poll_fd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) return false;

    char buffer[4096];
    ssize_t count = ::read(socket_.get(), buffer, sizeof(buffer));
    if (count <= 0) return false;
    parser_.append(std::string_view(buffer, static_cast<std::size_t>(count)));
    return true;
}

bool Connection::write_all(std::string_view data) {
    std::size_t sent = 0;
    while (sent < data.size()) {
        ssize_t count = ::write(socket_.get(), data.data() + sent, data.size() - sent);
        if (count < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (count == 0) return false;
        sent += static_cast<std::size_t>(count);
    }
    return true;
}
