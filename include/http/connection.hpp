#pragma once

#include "http/response.hpp"
#include "http/router.hpp"
#include <string>

namespace tyga::http {
class HttpConnection {
public:
  HttpConnection(int fd, Router &router);
  void run();
  bool read();
  bool process();

private:
  void send_response(Response &response);
  int fd_;
  std::string read_buffer_;
  std::string write_buffer_;
  std::size_t write_offset_ = 0;
  tyga::http::Router &router_;
};
} // namespace tyga::http