cmake_minimum_required(VERSION 3.20)
project(http_threadpool_server VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

add_library(server_core STATIC
    src/net/socket.cpp
    src/net/listener.cpp
    src/thread_pool/thread_pool.cpp
    src/http/response.cpp
    src/http/parser.cpp
    src/routing/router.cpp
    src/server/connection.cpp
    src/server/server.cpp
)
target_include_directories(server_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(server_core PUBLIC Threads::Threads nlohmann_json::nlohmann_json)

add_executable(server src/main.cpp)
target_link_libraries(server PRIVATE server_core)

enable_testing()
add_executable(unit_tests tests/test_main.cpp)
target_link_libraries(unit_tests PRIVATE server_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(unit_tests)
