#pragma once

#include "../http/parser.hpp"
#include "../net/socket.hpp"
#include "../routing/router.hpp"

#include <string_view>

class Connection {
public:
    Connection(Socket socket, const Router& router);
    void run();

private:
    bool read_more();
    bool write_all(std::string_view data);

    Socket socket_;
    const Router& router_;
    Parser parser_;
};
