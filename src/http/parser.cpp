#include "parser.hpp"

#include <cctype>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {

std::string to_lower(std::string_view text) {
    std::string out(text);
    for (char& c : out) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return out;
}

}

void Parser::append(std::string_view data) {
    buffer_.append(data.data(), data.size());
}

void Parser::reset() {
    buffer_.clear();
    consumed_ = 0;
    request_ = Request{};
}

void Parser::discard_consumed() {
    if (consumed_ > 0 && consumed_ <= buffer_.size()) {
        buffer_.erase(0, consumed_);
    }
    consumed_ = 0;
}

bool Parser::parse_request_line(std::string_view line, Request& request) {
    std::size_t first = line.find(' ');
    if (first == std::string_view::npos) return false;
    std::size_t second = line.find(' ', first + 1);
    if (second == std::string_view::npos) return false;
    std::string_view method = line.substr(0, first);
    std::string_view target = line.substr(first + 1, second - first - 1);
    std::string_view version = line.substr(second + 1);
    if (method.empty() || target.empty() || version.empty()) return false;
    if (target.front() != '/') return false;
    if (version != "HTTP/1.0" && version != "HTTP/1.1") return false;
    request.method = std::string(method);
    request.target = std::string(target);
    request.version = std::string(version);
    return true;
}

bool Parser::parse_header_line(std::string_view line, Request& request) {
    std::size_t colon = line.find(':');
    if (colon == std::string_view::npos || colon == 0) return false;
    std::string name = to_lower(line.substr(0, colon));
    std::string_view value = line.substr(colon + 1);
    std::size_t begin = value.find_first_not_of(" \t");
    std::size_t end = value.find_last_not_of(" \t");
    std::string trimmed;
    if (begin != std::string_view::npos) {
        trimmed = std::string(value.substr(begin, end - begin + 1));
    }
    request.headers[std::move(name)] = std::move(trimmed);
    return true;
}

ParseStatus Parser::parse() {
    request_ = Request{};
    consumed_ = 0;

    std::size_t line_end = buffer_.find("\r\n");
    if (line_end == std::string::npos) {
        return buffer_.size() > kMaxLine ? ParseStatus::Error : ParseStatus::NeedMore;
    }

    std::string_view request_line(buffer_.data(), line_end);
    if (!parse_request_line(request_line, request_)) return ParseStatus::Error;

    std::size_t headers_end = buffer_.find("\r\n\r\n", line_end + 2);
    if (headers_end == std::string::npos) {
        return buffer_.size() > kMaxHeaders ? ParseStatus::Error : ParseStatus::NeedMore;
    }

    std::size_t pos = line_end + 2;
    while (pos < headers_end) {
        std::size_t newline = buffer_.find("\r\n", pos);
        if (newline == std::string::npos || newline > headers_end) return ParseStatus::Error;
        std::string_view header_line(buffer_.data() + pos, newline - pos);
        if (!parse_header_line(header_line, request_)) return ParseStatus::Error;
        pos = newline + 2;
    }

    if (request_.headers.count("transfer-encoding") != 0) return ParseStatus::Error;

    std::size_t content_length = 0;
    auto length_it = request_.headers.find("content-length");
    if (length_it != request_.headers.end()) {
        const std::string& value = length_it->second;
        if (value.empty()) return ParseStatus::Error;
        for (char c : value) {
            if (c < '0' || c > '9') return ParseStatus::Error;
        }
        try {
            content_length = std::stoull(value);
        } catch (...) {
            return ParseStatus::Error;
        }
        if (content_length > kMaxBody) return ParseStatus::Error;
    }

    std::size_t body_start = headers_end + 4;
    if (buffer_.size() - body_start < content_length) return ParseStatus::NeedMore;

    request_.body = buffer_.substr(body_start, content_length);
    consumed_ = body_start + content_length;

    request_.keep_alive = (request_.version == "HTTP/1.1");
    auto connection_it = request_.headers.find("connection");
    if (connection_it != request_.headers.end()) {
        std::string value = to_lower(connection_it->second);
        if (value == "close") {
            request_.keep_alive = false;
        } else if (value == "keep-alive") {
            request_.keep_alive = true;
        }
    }

    return ParseStatus::Complete;
}
