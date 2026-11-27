#pragma once

#include "../net/listener.hpp"
#include "../routing/router.hpp"
#include "../thread_pool/thread_pool.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>

class Server {
public:
    Server(std::uint16_t port, std::size_t threads, int signal_fd);
    void run();

private:
    Listener listener_;
    ThreadPool pool_;
    Router router_;
    int signal_fd_;
    std::atomic<bool> running_{true};
};
