cmake_minimum_required(VERSION 3.20)
project(http_threadpool_server VERSION 0.1.0 LANGUAGES CXX)

# C++ standard requirements
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# FetchContent for external dependencies
include(FetchContent)

# Compiler warning flags
add_compile_options(-Wall -Wextra -Wpedantic)

# Threads (pthreads)
find_package(Threads REQUIRED)

# nlohmann/json for working with JSON in endpoints
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

# GoogleTest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Main server library
add_library(server_core STATIC
    # ...
)
target_include_directories(server_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(server_core PUBLIC Threads::Threads nlohmann_json::nlohmann_json)
set_target_properties(server_core PROPERTIES LINKER_LANGUAGE CXX)

# Main executable: ./build/server
add_executable(server src/main.cpp)
target_link_libraries(server PRIVATE server_core)

# Unit tests
enable_testing()
add_executable(unit_tests tests/test_main.cpp)
target_link_libraries(unit_tests PRIVATE server_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(unit_tests)
