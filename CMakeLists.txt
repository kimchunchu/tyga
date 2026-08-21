cmake_minimum_required(VERSION 3.16)

project(tyga VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(tyga
src/app.cpp
src/core/threadpool.cpp
src/http/connection.cpp
src/http/parser.cpp
src/http/request.cpp
src/http/response.cpp
src/http/router.cpp
src/http/string.cpp
src/net/server.cpp
)

# add_executable(tyga_server
#     examples/main.cpp
# )

# target_link_libraries(tyga_server
#     PRIVATE tyga
# )

target_include_directories(tyga PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/include
)

# target_compile_options(tyga PRIVATE
#     -fsanitize=address
#     -fno-omit-frame-pointer
# )

# target_link_options(tyga PRIVATE
#     -fsanitize=address
# )