#include "http/connection.hpp"
#include "http/parser.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include <cerrno>
#include <chrono>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

constexpr size_t MAX_REQUEST_SIZE = 1024 * 1024;

namespace tyga::http {
HttpConnection::HttpConnection(int fd, Router &router)
    : fd_(fd), router_(router),
      last_activity_(std::chrono::steady_clock::now()) {};

bool HttpConnection::process() {
  while (true) {
    ParseResult result = parse_request(read_buffer_);
    if (result.status == ParseStatus::Incomplete) {
      return true;
    }

    if (result.status == ParseStatus::Error) {
      Response response = Response::bad_request().header("Connection", "close");
      send_response(response);

      close_after_write_ = true;
      return true;
    }

    Request &request = *result.request;
    Response response = router_.handle(request);
    bool should_close = request.should_close();

    if (should_close) {
      close_after_write_ = true;
      response.header("Connection", "close");
    }

    send_response(response);
    read_buffer_.erase(0, result.consumed);

    if (should_close) {
      return true;
    }
  }
}

bool HttpConnection::read() {
  char buffer[4096];
  ssize_t bytes_read = ::read(fd_, buffer, sizeof(buffer));

  if (bytes_read > 0) {
    last_activity_ = std::chrono::steady_clock::now();
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

bool HttpConnection::write() {
  while (write_offset_ < write_buffer_.size()) {
    ssize_t bytes_written = ::write(fd_, write_buffer_.data() + write_offset_,
                                    write_buffer_.size() - write_offset_);

    if (bytes_written > 0) {
      last_activity_ = std::chrono::steady_clock::now();
      write_offset_ += bytes_written;
      continue;
    }

    if (bytes_written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      return true;
    }

    return false;
  }

  write_buffer_.clear();
  write_offset_ = 0;

  if (close_after_write_) {
    return false;
  }

  return true;
}

bool HttpConnection::wants_write() const {
  return write_offset_ < write_buffer_.size();
}

void HttpConnection::send_response(Response &response) {
  write_buffer_ += response.serialize();
}

bool HttpConnection::is_idle_timeout() const {
  return std::chrono::steady_clock::now() - last_activity_ >
         std::chrono::seconds(30);
}
} // namespace tyga::http