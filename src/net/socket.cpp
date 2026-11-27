#include "socket.hpp"

#include <unistd.h>

void Socket::reset() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}
