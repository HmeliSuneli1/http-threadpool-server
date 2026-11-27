#include "router.hpp"

Response Router::route(const Request& request) const {
    std::string path = request.path();

    if (path == "/health") {
        if (request.method == "GET") {
            Response response;
            response.status = 200;
            response.body = "{\"status\":\"ok\"}";
            return response;
        }
        Response response;
        response.status = 405;
        response.headers["Allow"] = "GET";
        return response;
    }

    Response response;
    response.status = 404;
    return response;
}
