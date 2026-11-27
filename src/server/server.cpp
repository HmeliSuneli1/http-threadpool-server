#include "server.hpp"

#include "connection.hpp"

#include <cerrno>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

Server::Server(std::uint16_t port, std::size_t threads, int signal_fd)
    : listener_(port, SOMAXCONN), pool_(threads), signal_fd_(signal_fd) {}

void Server::run() {
    while (running_.load()) {
        pollfd fds[2];
        fds[0].fd = listener_.fd();
        fds[0].events = POLLIN;
        fds[0].revents = 0;
        fds[1].fd = signal_fd_;
        fds[1].events = POLLIN;
        fds[1].revents = 0;

        int result = ::poll(fds, 2, -1);
        if (result < 0) {
            if (errno == EINTR) continue;
            break;
        }

        if ((fds[1].revents & POLLIN) != 0) {
            char buffer[64];
            while (::read(signal_fd_, buffer, sizeof(buffer)) > 0) {}
            running_.store(false);
            break;
        }

        if ((fds[0].revents & POLLIN) != 0) {
            Socket socket = listener_.accept();
            if (!socket.valid()) continue;
            if (!running_.load()) break;
            int fd = socket.release();
            pool_.submit([fd, this]() {
                Socket connection_socket(fd);
                Connection connection(std::move(connection_socket), router_);
                connection.run();
            });
        }
    }
    pool_.stop();
}
