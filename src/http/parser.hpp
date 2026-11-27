#pragma once

#include "request.hpp"

#include <cstddef>
#include <string>
#include <string_view>

enum class ParseStatus {
    NeedMore,
    Complete,
    Error
};

class Parser {
public:
    void append(std::string_view data);
    ParseStatus parse();
    const Request& request() const { return request_; }
    void discard_consumed();
    void reset();

private:
    static constexpr std::size_t kMaxLine = 8192;
    static constexpr std::size_t kMaxHeaders = 16384;
    static constexpr std::size_t kMaxBody = 1024 * 1024;

    bool parse_request_line(std::string_view line, Request& request);
    bool parse_header_line(std::string_view line, Request& request);

    std::string buffer_;
    std::size_t consumed_ = 0;
    Request request_;
};
