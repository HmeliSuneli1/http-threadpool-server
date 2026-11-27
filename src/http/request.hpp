#pragma once

#include <cstddef>
#include <map>
#include <string>

struct Request {
    std::string method;
    std::string target;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    bool keep_alive = true;

    std::string path() const {
        std::size_t query_pos = target.find('?');
        if (query_pos == std::string::npos) return target;
        return target.substr(0, query_pos);
    }

    std::string query() const {
        std::size_t query_pos = target.find('?');
        if (query_pos == std::string::npos) return std::string();
        return target.substr(query_pos + 1);
    }
};
