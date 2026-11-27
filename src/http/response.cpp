#include "response.hpp"

#include <string>

std::string status_text(int status) {
    switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 409: return "Conflict";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}

std::string serialize_response(const Response& response, bool keep_alive) {
    std::string out;
    out.reserve(response.body.size() + 256);
    out += "HTTP/1.1 ";
    out += std::to_string(response.status);
    out += ' ';
    out += status_text(response.status);
    out += "\r\n";
    if (!response.body.empty()) {
        out += "Content-Type: application/json\r\n";
    }
    out += "Content-Length: ";
    out += std::to_string(response.body.size());
    out += "\r\n";
    for (const auto& entry : response.headers) {
        out += entry.first;
        out += ": ";
        out += entry.second;
        out += "\r\n";
    }
    out += keep_alive ? "Connection: keep-alive\r\n" : "Connection: close\r\n";
    out += "\r\n";
    out += response.body;
    return out;
}
