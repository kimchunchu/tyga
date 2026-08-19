# Tyga

Tyga is a lightweight HTTP/1.1 server written from scratch in C++20.

The project is built without relying on existing HTTP server frameworks. Its purpose is to understand how an HTTP server works internally, from TCP sockets and connection handling to HTTP parsing, routing, request handling, and response generation.

## Features

* TCP socket server
* HTTP/1.1 request parsing
* Request-Line parsing
* HTTP header parsing
* Request body parsing
* Content-Length handling
* Query parameters
* URL decoding
* Path parameters
* Case-insensitive HTTP header lookup
* HTTP response generation
* Automatic Content-Length generation
* HTTP/1.1 persistent connections
* `Connection: close` support
* HTTP request routing
* Thread pool based connection handling
* Application-level API

## Architecture

```text
App
 |
 +-- Router
 |
 +-- ThreadPool
 |
 +-- Server
      |
      +-- accept()
      |
      +-- HttpConnection
            |
            +-- TCP read
            |
            +-- Request Parser
            |
            +-- Router
            |
            +-- Handler
            |
            +-- Response
```

The main components are separated by responsibility.

### App

`App` is the top-level application object. It owns the main components of the server and provides the public API used to configure routes and start the server.

```cpp
App app;

app.get("/", [](const Request&) {
    return Response::ok("Hello World!");
});

app.run();
```

### Server

`Server` manages the listening socket and accepts incoming TCP connections.

Its main responsibilities are:

* Creating the socket
* Binding to an address and port
* Listening for connections
* Accepting clients
* Creating HTTP connections

### HttpConnection

`HttpConnection` represents a single client connection.

It manages:

* Reading from the TCP socket
* Maintaining the receive buffer
* Parsing HTTP requests
* Routing requests
* Sending responses
* Persistent connections
* Connection termination

### Parser

The HTTP parser converts raw TCP data into a `Request`.

```text
TCP Buffer
    |
    v
HTTP Parser
    |
    v
Request
```

The parser supports incremental input, meaning an HTTP request does not have to arrive in a single `read()` call.

For example:

```text
read #1
    |
    +-- incomplete request
    |
    v
buffer

read #2
    |
    +-- append
    |
    v
complete request
```

### Router

The router maps HTTP methods and paths to handlers.

Example:

```cpp
app.get("/users", [](const Request& request) {
    return Response::ok("Users");
});

app.get("/users/:id", [](const Request& request) {
    return Response::ok("User");
});
```

Path parameters can be extracted from routes such as:

```text
/users/:id
```

with requests such as:

```text
/users/123
```

### ThreadPool

The thread pool provides worker threads for handling connections without creating a new thread for every connection.

The `ThreadPool` is owned by the application and passed by reference to components that need it.

## Request

A parsed request contains:

```text
Request
├── method
├── path
├── query
├── version
├── headers
├── params
└── body
```

Example request:

```http
POST /users?debug=true HTTP/1.1
Host: localhost:8080
Content-Type: application/json
Content-Length: 15

{"name":"John"}
```

The application can access its data through the `Request` API.

```cpp
auto content_type = request.header("Content-Type");
auto debug = request.query_param("debug");

std::cout << request.body;
```

## Response

Handlers return a `Response` object.

```cpp
app.get("/", [](const Request&) {
    return Response::ok("Hello World!");
});
```

The server automatically generates the `Content-Length` header when necessary.

The resulting response is similar to:

```http
HTTP/1.1 200 OK
Content-Length: 12

Hello World!
```

## HTTP/1.1 Persistent Connections

Tyga supports HTTP/1.1 persistent connections.

HTTP/1.1 connections remain open by default.

```http
GET / HTTP/1.1
Host: localhost:8080
```

The connection can explicitly be closed using:

```http
GET / HTTP/1.1
Host: localhost:8080
Connection: close
```

The server sends the response and closes the socket after the response has been transmitted.

## Multiple Requests per Connection

A single TCP read is not assumed to contain exactly one HTTP request.

Tyga maintains a receive buffer and continues parsing until the buffer contains an incomplete request.

```text
TCP stream
    |
    v
Receive Buffer
    |
    +-- Request A
    |
    +-- Request B
    |
    +-- Partial Request C
```

Completed requests are processed immediately while incomplete data remains in the buffer until the next read.

## Project Structure

```text
tyga/
├── CMakeLists.txt
├── include/
│   └── tyga/
│       ├── app.hpp
│       │
│       ├── http/
│       │   ├── request.hpp
│       │   ├── response.hpp
│       │   ├── header.hpp
│       │   ├── method.hpp
│       │   ├── parser.hpp
│       │   ├── router.hpp
│       │   └── string.hpp
│       │
│       ├── net/
│       │   ├── server.hpp
│       │   ├── connection.hpp
│       │   └── socket.hpp
│       │
│       └── core/
│           └── thread_pool.hpp
│
├── src/
│   ├── main.cpp
│   ├── app.cpp
│   │
│   ├── http/
│   │   ├── request.cpp
│   │   ├── response.cpp
│   │   ├── parser.cpp
│   │   ├── router.cpp
│   │   └── string.cpp
│   │
│   ├── net/
│   │   ├── server.cpp
│   │   └── connection.cpp
│   │
│   └── core/
│       └── thread_pool.cpp
│
└── tests/
```

## Requirements

* C++20
* CMake 3.16 or later
* POSIX-compatible operating system

The project is currently developed and tested on macOS and is intended to remain portable to Linux.

## Build

Clone the repository and build with CMake:

```bash
git clone <repository-url>
cd tyga

cmake -S . -B build
cmake --build build
```

Run the server:

```bash
./build/tyga
```

The default server listens on port `8080`.

## Example

A minimal application:

```cpp
#include "app.hpp"

int main() {
    App app;

    app.get("/", [](const Request&) {
        return Response::ok("Hello World!");
    });

    app.get("/users/:id", [](const Request& request) {
        return Response::ok("User");
    });

    app.run();

    return 0;
}
```

Test it with:

```bash
curl http://localhost:8080/
```

Or:

```bash
curl http://localhost:8080/users/123
```

## Version

Current version:

```text
v1.0.0
```

`v1.0.0` represents the first complete implementation of the core HTTP server architecture.

## Roadmap

### v1.x

* Request size limits
* Header size limits
* Connection timeout
* HTTP error responses
* Graceful shutdown
* More comprehensive tests
* Middleware
* JSON request and response support
* Static file serving

### v2.x

The next major architectural goal is to move from a thread-per-connection model toward an event-driven architecture.

Potential components include:

* Non-blocking sockets
* `epoll` on Linux
* `kqueue` on macOS
* Event loop
* Connection state machines
* More scalable concurrent request processing

## Goals

Tyga is primarily an educational and engineering project focused on understanding the internals of network servers and modern C++.

The project intentionally starts from low-level primitives instead of hiding networking and HTTP behavior behind a framework.

The long-term goal is to evolve Tyga from a simple HTTP server into a more complete systems programming project while keeping the architecture understandable and lightweight.
