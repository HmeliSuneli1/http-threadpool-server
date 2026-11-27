#pragma once

#include <map>
#include <string>

struct Response {
    int status = 200;
    std::map<std::string, std::string> headers;
    std::string body;
};

std::string status_text(int status);
std::string serialize_response(const Response& response, bool keep_alive);
