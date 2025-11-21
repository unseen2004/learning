cmake_minimum_required(VERSION 3.16)
project(webserver_cache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

find_package(Boost 1.70 REQUIRED COMPONENTS system)

option(ENABLE_RDMA "Enable RDMA fast path (requires rdma-core)" ON)

add_executable(webserver
        src/cpp/main.cpp
        src/headers/server.hpp
        src/cpp/server.cpp
        src/cpp/session.cpp
        src/headers/session.hpp
        src/cpp/signals.cpp
        src/headers/signals.hpp
        src/cpp/util/config.cpp
        src/headers/util/config.hpp
        src/cpp/util/logging.cpp
        src/headers/util/logging.hpp
        src/cpp/util/time.cpp
        src/headers/util/time.hpp
        src/cpp/util/metrics.cpp
        src/headers/util/metrics.hpp
        src/headers/http/headers.hpp
        src/headers/http/mime.hpp
        src/cpp/http/mime.cpp
        src/cpp/http/request.cpp
        src/headers/http/request.hpp
        src/headers/http/response.hpp
        src/cpp/http/parser.cpp
        src/cpp/http/parser.cpp
        src/cpp/fs/path_utils.cpp
        src/headers/fs/path_utils.hpp
        src/cpp/fs/file_reader.cpp
        src/headers/fs/file_reader.hpp
        src/cpp/cache/lru_cache.cpp
        src/headers/cache/lru_cache.hpp
        src/cpp/rdma/protocol.cpp
        src/headers/rdma/protocol.hpp
        src/cpp/rdma/connection.cpp
        src/headers/rdma/connection.hpp
        src/cpp/rdma/rdma_server.cpp
        src/headers/rdma/rdma_server.hpp
        src/cpp/cache/lru_cache.cpp
        src/headers/http/parser.hpp
)

target_include_directories(webserver PRIVATE
        ${Boost_INCLUDE_DIRS}
        src
)

target_link_libraries(webserver
        PRIVATE
        Boost::system
        fmt::fmt
)

if (ENABLE_RDMA)
    target_sources(webserver PRIVATE
            src/cpp/rdma/rdma_server.cpp
            src/cpp/rdma/connection.cpp
            src/cpp/rdma/protocol.cpp
    )
    target_link_libraries(webserver PRIVATE rdmacm ibverbs)
    target_compile_definitions(webserver PRIVATE ENABLE_RDMA=1)
endif ()

if (UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(webserver PRIVATE Threads::Threads)
endif ()

if (MSVC)
    target_compile_options(webserver PRIVATE /W4 /permissive-)
else ()
    target_compile_options(webserver PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wno-sign-conversion)
endif ()