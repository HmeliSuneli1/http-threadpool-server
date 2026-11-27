#include "listener.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

Listener::Listener(std::uint16_t port, int backlog) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        throw std::runtime_error(std::string("socket: ") + std::strerror(errno));
    }

    int reuse = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (::bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        int error = errno;
        ::close(fd);
        throw std::runtime_error(std::string("bind: ") + std::strerror(error));
    }
    if (::listen(fd, backlog) != 0) {
        int error = errno;
        ::close(fd);
        throw std::runtime_error(std::string("listen: ") + std::strerror(error));
    }

    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags >= 0) {
        ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    fd_ = Socket(fd);
}

int Listener::fd() const {
    return fd_.get();
}

Socket Listener::accept() {
    int fd = ::accept(fd_.get(), nullptr, nullptr);
    if (fd < 0) {
        return Socket();
    }
    return Socket(fd);
}
