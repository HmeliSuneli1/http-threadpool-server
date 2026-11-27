#pragma once

#include "socket.hpp"

#include <cstdint>

class Listener {
public:
    Listener(std::uint16_t port, int backlog);
    int fd() const;
    Socket accept();

private:
    Socket fd_;
};
