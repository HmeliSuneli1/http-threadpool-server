#pragma once

#include "../http/request.hpp"
#include "../http/response.hpp"

class Router {
public:
    Response route(const Request& request) const;
};
