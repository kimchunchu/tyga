#include "http/connection.hpp"
#include "http/parser.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include <cerrno>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

constexpr size_t MAX_REQUEST_SIZE = 1024 * 1024;

namespace tyga::http {
HttpConnection::HttpConnection(int fd, Router &router)
    : fd_(fd), router_(router) {};

void HttpConnection::process() {
  while (true) {
    ParseResult result = parse_request(buffer_);
    if (result.status == ParseStatus::Incomplete) {
      return;
    }

    if (result.status == ParseStatus::Error) {
      return;
    }

    Request &request = *result.request;
    Response response = router_.handle(request);
    bool should_close = request.should_close();
    if (should_close) {
      response.header("Connection", "close");
    }

    send_response(response);
    buffer_.erase(0, result.consumed);
    std::cout << "client_fd: " << fd_ << std::endl;

    if (should_close) {
      std::cout << "커넥션 종료" << std::endl;
      close(fd_);
      return;
    }
  }
}

void HttpConnection::run() {
  while (true) {
    char temp[4096];
    ssize_t bytes_read = read(fd_, temp, sizeof(temp));
    if (bytes_read == 0) {
      break;
    }

    if (bytes_read < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        Response response =
            Response::request_timeout().header("Connection", "close");
        send_response(response);
      }
      break;
    }

    buffer_.append(temp, bytes_read);

    if (buffer_.size() > MAX_REQUEST_SIZE) {
      Response response =
          Response::payload_too_large().header("Connection", "close");
      send_response(response);
      close(fd_);
      return;
    }

    process();
  }
}

bool HttpConnection::read() {
  char buffer[4096];
  ssize_t bytes_read = ::read(fd_, buffer, sizeof(buffer));

  if (bytes_read > 0) {
    buffer_.append(buffer, bytes_read);
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