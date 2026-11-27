#include "server/server.hpp"

#include <csignal>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <unistd.h>

namespace {

int g_signal_write_fd = -1;

void handle_signal(int) {
    char byte = 1;
    ssize_t ignored = ::write(g_signal_write_fd, &byte, 1);
    (void)ignored;
}

bool parse_unsigned(const std::string& text, std::uint64_t& value) {
    if (text.empty()) return false;
    for (char c : text) {
        if (c < '0' || c > '9') return false;
    }
    try {
        value = std::stoull(text);
    } catch (...) {
        return false;
    }
    return true;
}

}

int main(int argc, char** argv) {
    std::uint16_t port = 8080;
    std::size_t threads = 4;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port") {
            if (i + 1 >= argc) {
                std::cerr << "missing value for --port" << std::endl;
                return 1;
            }
            std::uint64_t value = 0;
            if (!parse_unsigned(argv[++i], value) || value == 0 || value > 65535) {
                std::cerr << "invalid --port value" << std::endl;
                return 1;
            }
            port = static_cast<std::uint16_t>(value);
        } else if (arg == "--threads") {
            if (i + 1 >= argc) {
                std::cerr << "missing value for --threads" << std::endl;
                return 1;
            }
            std::uint64_t value = 0;
            if (!parse_unsigned(argv[++i], value) || value == 0) {
                std::cerr << "invalid --threads value" << std::endl;
                return 1;
            }
            threads = static_cast<std::size_t>(value);
        } else {
            std::cerr << "usage: server --port <port> --threads <threads>" << std::endl;
            return 1;
        }
    }

    ::signal(SIGPIPE, SIG_IGN);

    int pipe_fd[2];
    if (::pipe(pipe_fd) != 0) {
        std::cerr << "failed to create signal pipe" << std::endl;
        return 1;
    }
    int flags = ::fcntl(pipe_fd[0], F_GETFL, 0);
    ::fcntl(pipe_fd[0], F_SETFL, flags | O_NONBLOCK);
    flags = ::fcntl(pipe_fd[1], F_GETFL, 0);
    ::fcntl(pipe_fd[1], F_SETFL, flags | O_NONBLOCK);
    g_signal_write_fd = pipe_fd[1];

    struct sigaction action{};
    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    ::sigaction(SIGINT, &action, nullptr);
    ::sigaction(SIGTERM, &action, nullptr);

    int exit_code = 0;
    try {
        Server server(port, threads, pipe_fd[0]);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit_code = 1;
    }

    ::close(pipe_fd[0]);
    ::close(pipe_fd[1]);
    return exit_code;
}
