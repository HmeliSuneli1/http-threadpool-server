#pragma once

class Socket {
public:
    Socket() = default;
    explicit Socket(int fd) : fd_(fd) {}
    Socket(Socket&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            reset();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    ~Socket() {
        reset();
    }

    int get() const { return fd_; }
    bool valid() const { return fd_ >= 0; }
    int release() {
        int fd = fd_;
        fd_ = -1;
        return fd;
    }
    void reset();

private:
    int fd_ = -1;
};
