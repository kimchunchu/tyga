#include "http/connection.hpp"
#include "http/parser.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include <cerrno>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

constexpr size_t MAX_REQUEST_SIZE = 1024 * 1024;

namespace tyga::http {
HttpConnection::HttpConnection(int fd, Router &router)
    : fd_(fd), router_(router) {};

bool HttpConnection::process() {
  while (true) {
    ParseResult result = parse_request(read_buffer_);
    if (result.status == ParseStatus::Incomplete) {
      return true;
    }

    if (result.status == ParseStatus::Error) {
      Response response = Response::bad_request().header("Connection", "close");

      send_response(response);
      return false;
    }

    Request &request = *result.request;
    Response response = router_.handle(request);
    bool should_close = request.should_close();
    if (should_close) {
      response.header("Connection", "close");
    }

    send_response(response);
    read_buffer_.erase(0, result.consumed);

    if (should_close) {
      return false;
    }
  }
}

bool HttpConnection::read() {
  char buffer[4096];
  ssize_t bytes_read = ::read(fd_, buffer, sizeof(buffer));

  if (bytes_read > 0) {
    read_buffer_.append(buffer, bytes_read);
    return true;
  }
  if (bytes_read == 0) {
    return false;
  }

  if (errno == EAGAIN || errno == EWOULDBLOCK) {
    return true;
  }

  return false;
}

void HttpConnection::send_response(Response &response) {
  std::string data = response.serialize();
  size_t total_sent = 0;
  while (total_sent < data.size()) {
    ssize_t sent =
        send(fd_, data.data() + total_sent, data.size() - total_sent, 0);
    if (sent <= 0) {
      return;
    }

    total_sent += static_cast<size_t>(sent);
  }
}
} // namespace tyga::http